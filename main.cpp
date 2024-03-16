#include <iostream>
#include "glad/glad.h"
#include "src/tool/exglfw.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/renderpass/DirectDisplayer.h"
#include "src/renderpass/Renderer.h"
#include "src/renderpass/Renderer2.h"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/tool/loader.h"
#include "src/texture/Skybox.h"
#include "src/instance/Camera.h"
#include <opencv2/opencv.hpp>
#include "src/TinyUI.h"
using namespace std;


/**
 * TODO
 * Remake 管线，后期mapping和降噪
 * 改正emission->material
 * 改正btdf
 * 修改shader reader
 * Ray hit
 * ReSTIR GI
 */

// 一些状态 ------
GLFWwindow *window;
Renderer *pass1;
DirectDisplayer *pass2;
Scene *scene;
Camera *camera;
Skybox* skybox;
uint frameCounter = 0;

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
		frameCounter = 0;
    }
}

void update(float dt) {

    scene->update();
    pass1->reload_sceneinfos(scene);

    static uint last_colorT = 0, last_wposT = 0;
	static bool fast_shade = true;
	static glm::mat4 back_projection(1);

	frameCounter++;

    // 渲染管线
	// ---------------------------------------
    {
        pass1->use();
        {
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "fast_shade"), fast_shade);
            pass1->bind_texture("skybox", skybox->textureObject, 0);
            pass1->bind_texture("skybox_samplecache", skybox->skyboxsamplerObject, 1);
            glUniformMatrix4fv(glGetUniformLocation(pass1->shaderProgram, "v2w_mat"), 1, GL_FALSE, glm::value_ptr(camera->v2w_matrix()));
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_H"), SCREEN_H);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "MAX_DEPTH"), 2);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SPP"), Config::SPP);
            glUniform1ui(glGetUniformLocation(pass1->shaderProgram, "frameCounter"), frameCounter);
            glUniform1f(glGetUniformLocation(pass1->shaderProgram, "skybox_Light_SUM"), skybox->lightSum);
            glUniform1f(glGetUniformLocation(pass1->shaderProgram, "fov"), SCREEN_FOV);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_W"), skybox->width);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_H"), skybox->height);
        }
        pass1->draw();

        pass2->use();
        {
            glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_H"), SCREEN_H);
        }
        pass2->draw(pass1->colorBufferSSBO);

//        glm::mat4 viewPort = glm::matbyrow({
//                                                   1./SCREEN_W, 0, 			0,	 0.5,
//                                                   0, 			 1./SCREEN_H, 	0,	 0.5,
//                                                   0, 			 0, 			0,	 0,
//                                                   0, 			 0, 			0,	 1
//                                           });
//        back_projection = viewPort * camera->projection() * camera->w2v_matrix();
//
//        pass_mix->use();
//        {
//            pass_mix->bind_texture("cur_colorT", pass1->attach_textures[0]);
//            pass_mix->bind_texture("cur_wposT", pass1->attach_textures[3]);
//            pass_mix->bind_texture("last_colorT", last_colorT);
//            pass_mix->bind_texture("last_wposT", last_wposT);
//            glUniform1ui(glGetUniformLocation(pass_mix->shaderProgram, "frameCounter"), frameCounter);
//            glUniformMatrix4fv(glGetUniformLocation(pass_mix->shaderProgram, "back_proj"), 1, GL_FALSE, glm::value_ptr(back_projection));
//            glUniform1i(glGetUniformLocation(pass_mix->shaderProgram, "is_motionvector_enabled"), Config::is_motionvector_enabled);
//        }
//        pass_mix->draw();
//
//        last_colorT = pass_mix->attach_textures[1];
//        last_wposT = pass1->attach_textures[3];
//
//        pass_fw->use();
//        {
//            glUniform1i(glGetUniformLocation(pass_fw->shaderProgram, "SCREEN_W"), SCREEN_W);
//            glUniform1i(glGetUniformLocation(pass_fw->shaderProgram, "SCREEN_H"), SCREEN_H);
//            glUniform1i(glGetUniformLocation(pass_fw->shaderProgram, "is_filter_enabled"), Config::is_filter_enabled);
//            pass_fw->bind_texture("prevpass_color", pass_mix->attach_textures[0]);
//            pass_fw->bind_texture("prevpass_albedo", pass1->attach_textures[1]);
//            pass_fw->bind_texture("prevpass_normal", pass1->attach_textures[2]);
//        }
//        pass_fw->draw();
//
//        pass_fh->use();
//        {
//            glUniform1i(glGetUniformLocation(pass_fh->shaderProgram, "SCREEN_W"), SCREEN_W);
//            glUniform1i(glGetUniformLocation(pass_fh->shaderProgram, "SCREEN_H"), SCREEN_H);
//            glUniform1i(glGetUniformLocation(pass_fh->shaderProgram, "is_filter_enabled"), Config::is_filter_enabled);
//            pass_fh->bind_texture("prevpass_color", pass_fw->attach_textures[0]);
//            pass_fh->bind_texture("prevpass_albedo", pass1->attach_textures[1]);
//            pass_fh->bind_texture("prevpass_normal", pass1->attach_textures[2]);
//        }
//        pass_fh->draw();
    }
    //--------------------------------------

	if(glfwGetKeyDown(window, GLFW_KEY_R)) fast_shade = !fast_shade, frameCounter = 0;

	// Menu
	if(glfwGetKeyDown(window, GLFW_KEY_E)) {
//		show_imgui = show_imgui ^ 1;
	}
	// Output camera pose
	if(glfwGetKeyDown(window, GLFW_KEY_P)) {
		Transform t = camera->transform;
		cout << "Camera pose:\n";
		cout << "Position: " << t.position[0] << ", " << t.position[1] << ", " << t.position[2] << '\n';
		cout << "Rotation: " << t.rotation[0] << ", " << t.rotation[1] << ", " << t.rotation[2] << '\n';
	}

	Transform before = camera->transform;

	// screen shot.
	if(glfwGetKeyDown(window, GLFW_KEY_T)) {
        string file_path = str_format("screenshots/%s.png", localtimestring().c_str());
        cv::Mat screenshot(SCREEN_W, SCREEN_H, CV_8UC3);
        glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, screenshot.data);
        cv::Mat fliped;
        cv::flip(screenshot, fliped, 0);
        cv::imwrite(file_path, fliped);

        cout << "ScreenShot " << file_path << '\n';
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
	if(glfwGetKey(window, GLFW_KEY_Q)) glfwSetWindowShouldClose(window, GL_TRUE);

	if(!(camera->transform == before)) frameCounter = 0;
}

void init_scene() {

    // passes
    pass1    = new Renderer("shader/pathtracing.glsl");
    pass2    = new DirectDisplayer("shader/postprocessing/direct.frag");
//    pass_mix = new RenderPass("shader/postprocessing/mixAndMap.frag", 0, true);

//    pass1    = new Renderer("shader/pathtracing2024.frag", 4);
//    pass_mix = new RenderPass("shader/postprocessing/mixAndMap.frag", 2);
//    pass_fw  = new RenderPass("shader/postprocessing/filter_w.frag", 1);
//	pass_fh  = new RenderPass("shader/postprocessing/filter_h.frag", 0, true);

    scene = new Scene("Scene");
    camera = new Camera(SCREEN_FOV);
    {
        Instance *o1 = AssimpLoader::load_model("model/casa_obj.glb");
        o1->transform.rotation = vec3(-90, 0, 0);
		// pre setting
//		Material *m1 = o1->get_child(0)->get_child(1)->meshes[0]->material;
//		m1->roughness = 0.01;
//		m1->metallic = 0;
//		m1->index_of_refraction = 1.25;
//		m1->spec_trans = 1;
//		Material *m2 = o1->get_child(0)->get_child(3)->meshes[0]->material;
//		m2->roughness = 0.04;
//		m2->metallic = 0;
//		m2->index_of_refraction = 1.01;
//		m2->spec_trans = 0.8;
		scene->add_child(o1);

//        Instance *o2 = AssimpLoader::load_model("model/casa_obj.glb");
//        o2->transform.rotation = vec3(-M_PI / 2, -M_PI / 2, 0);
//        scene->add_child(o2);

//        Instance *light= AssimpLoader::load_model("model/light.obj");
//        light->transform.scale = vec3(30, 30, 30);
//        light->transform.position = vec3(0, 100, 0);
//        light->get_child(0)->meshes[0]->material->emission = vec3(5);
//        light->get_child(0)->meshes[0]->material->is_emit = true;
//        scene->add_child(light);
    }

	skybox = new Skybox("hdrs/kloofendal_48d_partly_cloudy_puresky_2k.hdr");

    camera->transform.rotation.y = 180;
	camera->transform.position = vec3(-12.1396, 9.27221, 13.2912);
	camera->transform.rotation = vec3(-26.19, -45.8484, 0);
    scene->update();
    pass1->reload_scene(scene);
    std::cout << "BVH size:" << scene->sceneBVHRoot->siz << std::endl;
    std::cout << "BVH depth:" << scene->sceneBVHRoot->depth << std::endl;
}

int main(int argc, const char* argv[]) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);//次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心渲染模式

    //创建窗口，放入上下文中
    window = glfwCreateWindow(WINDOW_W, WINDOW_H, "My Window", NULL, NULL);
    glfwSetWindowPos(window, 500, 200);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);

    //初始化glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glDisable(GL_DEPTH_TEST);

    TinyUI::init(window);
    init_scene();


    float last_time = glfwGetTime(), detaTime;
	float fps = 60, counter_time = 0, counter_frame = 0;
    while(!glfwWindowShouldClose(window)) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        detaTime = glfwGetTime() - last_time;
        last_time += detaTime;

        counter_frame ++;
        counter_time += detaTime;
        if(counter_time > 1) {
            fps = counter_frame / counter_time;
            counter_frame = counter_time = 0;
        }

        glViewport(0, WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H);
        update(detaTime);

        glViewport(0, 0, WINDOW_W, WINDOW_H);
        TinyUI::update(scene, fps);

        glfwSwapBuffers(window); //交换两层颜色缓冲
        glfwPollEvents();	//检查有没有发生事件，调用相应回调函数
    }

    TinyUI::terminate();
    glfwTerminate();
    return 0;
}