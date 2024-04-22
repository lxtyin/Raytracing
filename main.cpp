#include <iostream>
#include "glad/glad.h"
#include "src/tool/exglfw.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/renderpass/DirectDisplayer.h"
#include "src/renderpass/RasterPass.h"
#include "src/renderpass/Renderer.h"
#include "src/renderpass/ToneMappingGamma.h"
#include "src/renderpass/TAA.h"
#include "src/renderpass/SVGFTemporalFilter.h"
#include "src/renderpass/SVGFSpatialFilterPass.h"
#include "src/renderpass/SVGFMergePass.h"
#include "src/renderpass/StaticBlender.h"
#include "src/ResourceManager.h"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/tool/loader.h"
#include "src/texture/Skybox.h"
#include "src/instance/Camera.h"
#include "src/BVH.h"
#include <opencv2/opencv.hpp>
#include "src/TinyUI.h"
#include <chrono>
#include <random>
using namespace std;

/**
 * TODO
 * 降噪
 *    - TAA使用GBuffer Check，双线性插值？
 *    - Variance
 * 改正emission->material
 * 改正btdf
 * 修改shader reader
 * Ray hit
 * ReSTIR GI
 */

GLFWwindow *window;
Scene *scene;
Camera *camera;
Skybox* skybox;
uint frameCounter = 0;

RasterPass *rasterPass;
Renderer *renderPass;
SVGFSpatialFilterPass *svgfSpatialFilterPass;
SVGFTemporalFilter *svgfTemporalFilterPass;
SVGFMergePass *svgfMergePass;
ToneMappingGamma *mappingPass;
TAA *taaPass;
StaticBlender *staticBlenderPass;
DirectDisplayer *directPass;

// ----

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
                            void *userParam)
{
    // 忽略一些不重要的错误/警告代码
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static double mouse_lastX = xpos, mouse_lastY = ypos;
    static int mouse_button = GLFW_RELEASE;
    int nw_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
    if(mouse_button == GLFW_RELEASE && nw_button == GLFW_PRESS) {
        mouse_lastX = xpos, mouse_lastY = ypos;
    }
    mouse_button = nw_button;
    if(mouse_button == GLFW_PRESS) {
        double dx = xpos - mouse_lastX;
        double dy = ypos - mouse_lastY;
        camera->transform.rotation += vec3(0, -dx * 0.3, 0);
        camera->transform.rotation += vec3(-dy * 0.3, 0, 0);
        mouse_lastX = xpos;
        mouse_lastY = ypos;
    }
}

void mouse_clickcalback(GLFWwindow* window, int button, int state, int mod) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y); // →x ↓y

        if(x >= SCREEN_W || y >= SCREEN_H) return;

        float disz = SCREEN_W * 0.5 / tan(camera->fovX / 2);
        mat4 v2w = camera->v2w_matrix();
        vec3 ori = vec3(v2w * vec4(0, 0, 0, 1));
        vec3 dir = normalize(vec3(v2w * vec4(x - SCREEN_W / 2, SCREEN_H / 2 - y, -disz, 0)));
        Ray ray = Ray(ori, dir);

        Intersection isect;
        scene->sceneBVHRoot->rayIntersect(ray, isect);
        if(isect.exist) {
            TinyUI::selectInstance(isect.instancePtr);
        } else {
            TinyUI::selectInstance(nullptr);
        }
    }
}

void window_resize_callback(GLFWwindow* window, int w, int h) {
    Config::WINDOW_H = h;
    Config::WINDOW_W = w;
}

void update(float dt) {

    ResourceManager::manager->reload_scene(scene);

	static glm::mat4 back_projection = camera->projection() * camera->w2v_matrix();

	frameCounter++;

    // 渲染管线
	// ---------------------------------------
    {
        // TODO: support direct light filter.

        std::mt19937 mrand(0);
        for(int spp = 1;spp <= Config::SPP;spp++) {
            vec2 jitter = vec2(mrand() * 1.0f / mrand.max(), mrand() * 1.0f / mrand.max());
            rasterPass->use();
            rasterPass->draw(camera, jitter);

            renderPass->use();
            {
                renderPass->bind_texture("skybox", skybox->textureObject, 0);
                renderPass->bind_texture("skybox_samplecache", skybox->skyboxsamplerObject, 1);
                renderPass->bind_texture("depthGBufferTexture", rasterPass->depthGBufferTexture, 2);
                renderPass->bind_texture("normalGBufferTexture", rasterPass->normalGBufferTexture, 3);
                renderPass->bind_texture("uvGBufferTexture", rasterPass->uvGBufferTexture, 4);
                renderPass->bind_texture("instanceIndexGBufferTexture", rasterPass->instanceIndexGBufferTexture, 5);
                glUniform2f(glGetUniformLocation(renderPass->shaderProgram, "jitter"), jitter.x, jitter.y);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "currentspp"), spp);
                glUniform1f(glGetUniformLocation(renderPass->shaderProgram, "skybox_Light_SUM"), skybox->lightSum);
                glUniformMatrix4fv(glGetUniformLocation(renderPass->shaderProgram, "v2wMat"), 1, GL_FALSE, glm::value_ptr(camera->v2w_matrix()));
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "SCREEN_W"), SCREEN_W);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "SCREEN_H"), SCREEN_H);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "WINDOW_W"), Config::WINDOW_W);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "WINDOW_H"), Config::WINDOW_H);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "MAX_DEPTH"), 2);
                glUniform1f(glGetUniformLocation(renderPass->shaderProgram, "cameraNear"), camera->near);
                glUniform1f(glGetUniformLocation(renderPass->shaderProgram, "cameraFar"), camera->far);
                glUniform1ui(glGetUniformLocation(renderPass->shaderProgram, "frameCounter"), frameCounter);
                glUniform1f(glGetUniformLocation(renderPass->shaderProgram, "fov"), camera->fovX);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "SKY_W"), skybox->width);
                glUniform1i(glGetUniformLocation(renderPass->shaderProgram, "SKY_H"), skybox->height);
                glUniformMatrix4fv(glGetUniformLocation(renderPass->shaderProgram, "backprojMat"), 1, GL_FALSE, glm::value_ptr(back_projection));
            }
            renderPass->draw();
        }

        if(Config::useTemporalFilter) {
            svgfTemporalFilterPass->use();
            {
                glUniform1i(glGetUniformLocation(svgfTemporalFilterPass->shaderProgram, "SCREEN_W"), SCREEN_W);
                glUniform1i(glGetUniformLocation(svgfTemporalFilterPass->shaderProgram, "SCREEN_H"), SCREEN_H);
            }
            svgfTemporalFilterPass->draw(renderPass->indirectLumGBufferSSBO,
                                         renderPass->momentGBufferSSBO,
                                         renderPass->normalGBufferSSBO,
                                         renderPass->instanceIndexGBufferSSBO,
                                         renderPass->motionGBufferSSBO,
                                         renderPass->numSamplesGBufferSSBO);
        } else svgfTemporalFilterPass->firstFrame = true;

        // a'trous wavelet filter
        for(int i = 0;i < Config::filterLevel;i++) {
            svgfSpatialFilterPass->use();
            {
                glUniform1i(glGetUniformLocation(svgfSpatialFilterPass->shaderProgram, "SCREEN_W"), SCREEN_W);
                glUniform1i(glGetUniformLocation(svgfSpatialFilterPass->shaderProgram, "SCREEN_H"), SCREEN_H);
                glUniformMatrix4fv(glGetUniformLocation(svgfSpatialFilterPass->shaderProgram, "w2vMat"), 1, GL_FALSE, glm::value_ptr(camera->w2v_matrix()));
                glUniform1i(glGetUniformLocation(svgfSpatialFilterPass->shaderProgram, "step"), 1 << i);
            }
            svgfSpatialFilterPass->draw(
                    renderPass->indirectLumGBufferSSBO,
                    renderPass->normalGBufferSSBO,
                    renderPass->depthGBufferSSBO,
                    renderPass->momentGBufferSSBO,
                    renderPass->numSamplesGBufferSSBO);
        }

        svgfMergePass->use();
        {
            glUniform1i(glGetUniformLocation(svgfMergePass->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(svgfMergePass->shaderProgram, "SCREEN_H"), SCREEN_H);
        }
        svgfMergePass->draw(renderPass->directLumGBufferSSBO, renderPass->indirectLumGBufferSSBO, renderPass->albedoGBufferSSBO);

        if(Config::useStaticBlender) {
            staticBlenderPass->use();
            {
                glUniform1i(glGetUniformLocation(staticBlenderPass->shaderProgram, "SCREEN_W"), SCREEN_W);
                glUniform1i(glGetUniformLocation(staticBlenderPass->shaderProgram, "SCREEN_H"), SCREEN_H);
            }
            staticBlenderPass->draw(svgfMergePass->colorGBufferSSBO);
        }

        mappingPass->use();
        {
            glUniform1i(glGetUniformLocation(mappingPass->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(mappingPass->shaderProgram, "SCREEN_H"), SCREEN_H);
        }
        mappingPass->draw(svgfMergePass->colorGBufferSSBO);

        if(Config::useTAA) {
            taaPass->use();
            {
                glUniform1i(glGetUniformLocation(taaPass->shaderProgram, "SCREEN_W"), SCREEN_W);
                glUniform1i(glGetUniformLocation(taaPass->shaderProgram, "SCREEN_H"), SCREEN_H);
            }
            taaPass->draw(svgfMergePass->colorGBufferSSBO,
                          renderPass->motionGBufferSSBO,
                          renderPass->normalGBufferSSBO,
                          renderPass->instanceIndexGBufferSSBO);
        } else taaPass->firstFrame = true;

        directPass->use();
        {
            glUniform1i(glGetUniformLocation(directPass->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(directPass->shaderProgram, "SCREEN_H"), SCREEN_H);
            glUniform1i(glGetUniformLocation(directPass->shaderProgram, "selectedInstanceIndex"),
                        ResourceManager::manager->queryInstanceIndex(TinyUI::selectedInstance));
        }
        directPass->draw(svgfMergePass->colorGBufferSSBO,
                         renderPass->instanceIndexGBufferSSBO);

        back_projection = camera->projection() * camera->w2v_matrix();

    }
    //--------------------------------------

	// Output camera pose
	if(glfwGetKeyDown(window, GLFW_KEY_P)) {
		Transform t = camera->transform;
		cout << "Camera pose:\n";
		cout << "Position: " << t.position[0] << ", " << t.position[1] << ", " << t.position[2] << '\n';
		cout << "Rotation: " << t.rotation[0] << ", " << t.rotation[1] << ", " << t.rotation[2] << '\n';
	}

	Transform previousCameraTransform = camera->transform;

	// screen shot.
	if(glfwGetKeyDown(window, GLFW_KEY_T)) {
        string file_path = str_format("screenshots/%s.png", localtimestring().c_str());

        int framesize = SCREEN_H * SCREEN_W;
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderPass->colorGBufferSSBO); // TODO
        float* tmpdata = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, framesize * 3 * sizeof(float), GL_MAP_READ_BIT);
        std::vector<uchar> d(framesize * 3);
        for(int i = 0;i < framesize * 3;i++) d[i] = std::min(255, int(tmpdata[i] * 255));
        Texture t(SCREEN_W, SCREEN_H, 3, std::move(d));
        t.savephoto(file_path);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        cout << "ScreenShot " << file_path << std::endl;
    }

    float speed = 10;
    if(glfwGetKey(window, GLFW_KEY_W)) camera->transform.position += camera->transform.direction_z() * -speed * dt;
    if(glfwGetKey(window, GLFW_KEY_S)) camera->transform.position += camera->transform.direction_z() * speed * dt;
    if(glfwGetKey(window, GLFW_KEY_D)) camera->transform.position += camera->transform.direction_x() * speed * dt;
    if(glfwGetKey(window, GLFW_KEY_A)) camera->transform.position += camera->transform.direction_x() * -speed * dt;

	if(glfwGetKey(window, GLFW_KEY_LEFT)) camera->transform.rotation += vec3(0, 50, 0) * dt;
	if(glfwGetKey(window, GLFW_KEY_RIGHT)) camera->transform.rotation += vec3(0, -50, 0) * dt;
	if(glfwGetKey(window, GLFW_KEY_UP)) camera->transform.rotation += vec3(50, 0, 0) * dt;
	if(glfwGetKey(window, GLFW_KEY_DOWN)) camera->transform.rotation += vec3(-50, 0, 0) * dt;

    if(glfwGetKey(window, GLFW_KEY_SPACE)) camera->transform.position += vec3(0, speed * dt, 0);
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))  camera->transform.position += vec3(0, -speed * dt, 0);
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);

    // TODO: if updated.
    if(!(camera->transform == previousCameraTransform)) {
        staticBlenderPass->frameCounter = 0;
    }
}

void init_scene() {

    // passes
    rasterPass = new RasterPass("shader/rasterization/raster_vs.glsl", "shader/rasterization/raster_ps.glsl");
    renderPass    = new Renderer("shader/pathtracing.glsl");
    svgfTemporalFilterPass = new SVGFTemporalFilter("shader/postprocessing/SVGF_TemporalFilter.glsl");
    svgfSpatialFilterPass = new SVGFSpatialFilterPass("shader/postprocessing/SVGF_SpatialFilter.glsl");
    svgfMergePass = new SVGFMergePass("shader/postprocessing/SVGF_Merge.glsl");
    mappingPass    = new ToneMappingGamma("shader/postprocessing/ToneMappingGamma.glsl");
    taaPass    = new TAA("shader/postprocessing/TAA.glsl");
    directPass    = new DirectDisplayer("shader/postprocessing/direct.glsl");
    staticBlenderPass = new StaticBlender("shader/postprocessing/StaticBlender.glsl");

    ResourceManager::manager = new ResourceManager();

    Scene::main_scene = scene = new Scene("Scene");
    camera = new Camera(FOV_X, 1000);
    scene->add_child(camera);
    {
        Instance *o1 = AssimpLoader::load_model("model/casa_obj.glb");
        o1->transform.rotation = vec3(-90, 0, 0);
		scene->add_child(o1);

        Instance *light = new Instance("light");
        light->transform.rotation = vec3(63, 60, 0);
        light->emitterType = Emitter_DIRECTIONAL;
        light->emission = vec3(5, 5, 5);
        scene->add_child(light);
    }

	skybox = new Skybox("model/kloofendal_48d_partly_cloudy_puresky_2k.hdr");

    camera->transform.rotation.y = 180;
	camera->transform.position = vec3(-12.1396, 9.27221, 13.2912);
	camera->transform.rotation = vec3(-26.19, -45.8484, 0);

    ResourceManager::manager->reload_scene(scene);

    std::cout << "BVH size:" << scene->sceneBVHRoot->siz << std::endl;
    std::cout << "BVH depth:" << scene->sceneBVHRoot->depth << std::endl;
}

int main(int argc, const char* argv[]) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);//次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心渲染模式

    //创建窗口，放入上下文中
    window = glfwCreateWindow(Config::WINDOW_W, Config::WINDOW_H, "My Window", NULL, NULL);
    glfwSetWindowPos(window, 500, 200);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_clickcalback);
    glfwSetFramebufferSizeCallback(window, window_resize_callback);

    //初始化glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glDisable(GL_DEPTH);

    TinyUI::init(window);
    init_scene();

    float last_time = glfwGetTime(), detaTime;
	float fps = 60, counter_time = 0, counter_frame = 0;
    while(!glfwWindowShouldClose(window)) {

        detaTime = glfwGetTime() - last_time;
        last_time += detaTime;

        counter_frame ++;
        counter_time += detaTime;
        if(counter_time > 1) {
            fps = counter_frame / counter_time;
            counter_frame = counter_time = 0;
        }

        glViewport(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H);
        update(detaTime);

        glViewport(0, 0, Config::WINDOW_W, Config::WINDOW_H);
        TinyUI::update(scene, fps);

        glfwSwapBuffers(window); //交换两层颜色缓冲
        glfwPollEvents();	//检查有没有发生事件，调用相应回调函数
    }

    TinyUI::terminate();
    glfwTerminate();
    return 0;
}