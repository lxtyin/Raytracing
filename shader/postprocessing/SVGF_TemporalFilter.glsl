
// Temporal accumulation (in HDR).
// Update both current moment and color.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform bool firstFrame = false;

layout(binding = 0, std430) readonly buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float normalGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float instanceIndexGBuffer[];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    float motionGBuffer[];
};

layout(binding = 4, std430) writeonly buffer ssbo4 {
    float outputColorGBuffer[];
};
layout(binding = 5, std430) writeonly buffer ssbo5 {
    float outputMomentGBuffer[];
};
layout(binding = 6, std430) writeonly buffer ssbo6 {
    float outputNumSamplesGBuffer[];
};

layout(binding = 7, std430) readonly buffer ssbo7 {
    float historyColorGBuffer[];
};
layout(binding = 8, std430) readonly buffer ssbo8 {
    float historyMomentGBuffer[];
};
layout(binding = 9, std430) readonly buffer ssbo9 {
    float historyNumSamplesGBuffer[];
};

layout(binding = 10, std430) readonly buffer ssbo10 {
    float historynormalGBuffer[];
};
layout(binding = 11, std430) readonly buffer ssbo11 {
    float historyinstanceIndexGBuffer[];
};

bool geometry_test(uint ptr1, uint ptr2) {
    vec3 normal1 = vec3(
        normalGBuffer[ptr1 * 3 + 0],
        normalGBuffer[ptr1 * 3 + 1],
        normalGBuffer[ptr1 * 3 + 2]
    );
    float id1 = instanceIndexGBuffer[ptr1];
    vec3 normal2 = vec3(
        historynormalGBuffer[ptr2 * 3 + 0],
        historynormalGBuffer[ptr2 * 3 + 1],
        historynormalGBuffer[ptr2 * 3 + 2]
    );
    float id2 = historyinstanceIndexGBuffer[ptr2];

    return dot(normal1, normal2) > 0.9 && abs(id1 - id2) < 0.1;
}


bool interpolate(vec2 suv, uint testptr, out vec3 color, out vec2 moment) {
    ivec2 leftbottom = ivec2(floor(suv.x * SCREEN_W - 0.5),
                             floor(suv.y * SCREEN_H - 0.5));
    vec2 point = vec2(suv.x * SCREEN_W, suv.y * SCREEN_H);

    vec3 outcolor = vec3(0);
    vec2 outmoment = vec2(0);
    float totweight = 0;
    for(int i = 0;i < 4;i++) {
        ivec2 idx = leftbottom + ivec2(i / 2, i % 2);
        if(idx.x < 0 || idx.x >= SCREEN_W || idx.y < 0 || idx.y >= SCREEN_H) continue;
        int ptr = idx.y * SCREEN_W + idx.x;
        if(!geometry_test(testptr, ptr)) continue;

        float dx = abs(point.x - idx.x - 0.5);
        float dy = abs(point.y - idx.y - 0.5);
        float weight = max(0, (1 - dx) * (1 - dy));

        vec3 lastcolor = vec3(
            historyColorGBuffer[ptr * 3 + 0],
            historyColorGBuffer[ptr * 3 + 1],
            historyColorGBuffer[ptr * 3 + 2]
        );
        vec2 lastmoment = vec2(
            historyMomentGBuffer[ptr * 2 + 0],
            historyMomentGBuffer[ptr * 2 + 1]
        );
        outcolor += lastcolor * weight;
        outmoment += lastmoment * weight;
        totweight += weight;
    }
    color = outcolor / totweight;
    moment = outmoment / totweight;
    return totweight > 0.00001;
}

float luminance(vec3 c) {
    return c.x * 0.212671f + c.y * 0.715160f + c.z * 0.072169f;
}


void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 color = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    float lum = luminance(color);
    vec2 moment = vec2(lum, lum * lum);
    vec2 motion = vec2(
        motionGBuffer[pixelPtr * 2 + 0],
        motionGBuffer[pixelPtr * 2 + 1]
    );

    if(firstFrame) {
        outputColorGBuffer[pixelPtr * 3 + 0] = color.x;
        outputColorGBuffer[pixelPtr * 3 + 1] = color.y;
        outputColorGBuffer[pixelPtr * 3 + 2] = color.z;
        outputMomentGBuffer[pixelPtr * 2 + 0] = moment.x;
        outputMomentGBuffer[pixelPtr * 2 + 1] = moment.y;
        outputNumSamplesGBuffer[pixelPtr] = 1;
        return;
    }

    // variance guide test (in RGB space, HDR).
//    vec3 mu = vec3(0), var = vec3(0);
//    int nv = 0;
//    for(int i = 0;i < 9;i++) {
//        ivec2 index = pixelIndex + ivec2(i / 3 - 1, i % 3 - 1);
//        if(index.x < 0 || index.x >= SCREEN_W || index.y < 0 || index.y >= SCREEN_H) continue;
//        int ptr = index.y * SCREEN_W + index.x;
//        vec3 r = vec3(
//            colorGBuffer[ptr * 3 + 0],
//            colorGBuffer[ptr * 3 + 1],
//            colorGBuffer[ptr * 3 + 2]
//        );
//        nv++;
//        mu += r;
//        var += r * r;
//    }
//    mu /= nv;
//    var = max(vec3(0), var / nv - mu * mu);
//    vec3 sigma = sqrt(var);


    // Temporal accumulation
    vec2 last_uv = screen_uv - motion;
    if(last_uv.x >= 1 || last_uv.y >= 1 || last_uv.x < 0 || last_uv.y < 0) {
        outputNumSamplesGBuffer[pixelPtr] = 1;
    } else {
        ivec2 lastPixelIndex = ivec2(int(last_uv.x * SCREEN_W), int(last_uv.y * SCREEN_H));
        int lastPixelPtr = lastPixelIndex.y * SCREEN_W + lastPixelIndex.x;

        vec3 lastcolor;
        vec2 lastmoment;
        bool valid = interpolate(last_uv, pixelPtr, lastcolor, lastmoment);

        // No additional variance test
        if(valid) {
            float historyLen = historyNumSamplesGBuffer[lastPixelPtr];
            outputNumSamplesGBuffer[pixelPtr] = historyLen + 1;
//            color = mix(lastcolor, color, 1.0 / (historyLen + 1));
//            moment = mix(lastmoment, moment, 1.0 / (historyLen + 1));
            color = mix(lastcolor, color, 0.2);
            moment = mix(lastmoment, moment, 0.2);
        } else {
            outputNumSamplesGBuffer[pixelPtr] = 1;
        }

        // Additional variance test.
//        vec3 dir = lastcolor - mu;
//        if(!valid || abs(dir.x) > sigma.x || abs(dir.y) > sigma.y || abs(dir.z) > sigma.z) {
//            numSamplesGBuffer[pixelPtr] = 1;
//        } else {
//            float historyLen = historyNumSamplesGBuffer[lastPixelPtr];
//            numSamplesGBuffer[pixelPtr] = historyLen + 1;
////            color = mix(lastcolor, color, 1.0 / (historyLen + 1));
////            moment = mix(lastmoment, moment, 1.0 / (historyLen + 1));
//            color = mix(lastcolor, color, 0.2);
//            moment = mix(lastmoment, moment, 0.2);
//        }
    }

    outputColorGBuffer[pixelPtr * 3 + 0] = color.x;
    outputColorGBuffer[pixelPtr * 3 + 1] = color.y;
    outputColorGBuffer[pixelPtr * 3 + 2] = color.z;
    outputMomentGBuffer[pixelPtr * 2 + 0] = moment.x;
    outputMomentGBuffer[pixelPtr * 2 + 1] = moment.y;
}