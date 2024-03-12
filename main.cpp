#include <iostream>
#include "glad/glad.h"
#include "src/tool/exglfw.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/RenderPass.h"
#include "src/Renderer.h"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/tool/loader.h"
#include "src/texture/Skybox.h"
#include "src/instance/Camera.h"
#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#include "imgui/backend/imgui_impl_opengl3.h"
#include <opencv2/opencv.hpp>
using namespace std;

// 一些状态 ------
GLFWwindow *window;
Renderer *pass1;
RenderPass *pass_mix, *pass_fw, *pass_fh;
Scene *scene;
Camera *camera;
Skybox* skybox;
bool show_imgui = true;
uint frameCounter = 0;

// ----

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
            pass1->bind_texture("skybox", skybox->textureObject);
            pass1->bind_texture("skybox_samplecache", skybox->skyboxsamplerObject);
            glUniformMatrix4fv(glGetUniformLocation(pass1->shaderProgram, "v2w_mat"), 1, GL_FALSE, glm::value_ptr(camera->v2w_matrix()));
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_W"), SCREEN_W);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_H"), SCREEN_H);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SPP"), Config::SPP);
            glUniform1ui(glGetUniformLocation(pass1->shaderProgram, "frameCounter"), frameCounter);
            glUniform1f(glGetUniformLocation(pass1->shaderProgram, "skybox_Light_SUM"), skybox->lightSum);
            glUniform1f(glGetUniformLocation(pass1->shaderProgram, "fov"), SCREEN_FOV);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_W"), skybox->width);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_H"), skybox->height);
        }
        pass1->draw();

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
		show_imgui = show_imgui ^ 1;
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

void init() {

    // passes
    pass1    = new Renderer("shader/pathtracing2024.frag", 0, true);

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
    window = glfwCreateWindow(SCREEN_W, SCREEN_H, "My Window", NULL, NULL);
    glfwSetWindowPos(window, 500, 200);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);

    //初始化glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glDisable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    init();

    float last_time = glfwGetTime(), detaTime;
	float fps = 60, counter_time = 0, counter_frame = 0;
    while(!glfwWindowShouldClose(window)) {
        detaTime = glfwGetTime() - last_time;
        last_time += detaTime;
        glfwPollEvents();	//检查有没有发生事件，调用相应回调函数

        update(detaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        counter_frame ++;
        counter_time += detaTime;
		if(counter_time > 1) {
            fps = counter_frame / counter_time;
            counter_frame = counter_time = 0;
		}

//        ImGui::ShowDemoWindow(&show_imgui);
        if (show_imgui) {
            ImGui::Begin("Editor", &show_imgui);
			ImGui::Text(str_format("FPS: %.2f", fps).c_str());
			Config::insert_gui();
            scene->insert_gui();
            ImGui::End();
        }
        ImGui::Render();

		int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); //交换两层颜色缓冲
    }

    //释放资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}