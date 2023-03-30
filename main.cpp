#include <iostream>
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/RenderPass.h"
#include "src/Renderer.h"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/tool/loader.h"
#include "src/texture/HDRTexture.h"
#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#include "imgui/backend/imgui_impl_opengl3.h"
using namespace std;

// 一些状态 ------
GLFWwindow *window;
Renderer *pass1;
RenderPass *pass2, *pass3;
Scene *scene;
Instance *camera;
HDRTexture* skybox;

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
        camera->transform.rotation += vec3(0, dx * 0.01, 0);
        camera->transform.rotation += vec3(dy * 0.01, 0, 0);
        mouse_lastX = xpos;
        mouse_lastY = ypos;
    }
}

void update(float dt) {

    static uint frame = 1, last_frame_tex = 0;

    // 渲染管线
    pass1->use();
    {
        glUniformMatrix4fv(glGetUniformLocation(pass1->shaderProgram, "v2w_mat"), 1, GL_FALSE, glm::value_ptr(camera->matrix_to_global()));
        glUniform1ui(glGetUniformLocation(pass1->shaderProgram, "frameCounter"), frame++);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_W"), SCREEN_W);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_H"), SCREEN_H);
        if(glfwGetKey(window, GLFW_KEY_R)) {
            pass1->reload_meshes(scene);
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "fast_shade"), false);
        } else {
            frame = 1;
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "fast_shade"), true);
        }

        pass1->bind_texture("last_frame_texture", last_frame_tex);
        pass1->bind_texture("skybox", skybox->TTO);
        pass1->bind_texture("skybox_samplecache", skybox->sample_cache_tto);
        glUniform1f(glGetUniformLocation(pass1->shaderProgram, "skybox_Light_SUM"), skybox->Light_SUM);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_W"), skybox->width);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SKY_H"), skybox->height);
    }
    pass1->draw();

//    pass2->use();
//    {
//        glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_W"), SCREEN_W);
//        glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_H"), SCREEN_H);
//        pass2->bind_texture("prev_color", pass1->attach_textures[0]);
//        pass2->bind_texture("prev_albedo", pass1->attach_textures[1]);
//        pass2->bind_texture("prev_normal", pass1->attach_textures[2]);
//    }
//    pass2->draw();

    pass3->use();
    {
        glUniform1i(glGetUniformLocation(pass3->shaderProgram, "SCREEN_W"), SCREEN_W);
        glUniform1i(glGetUniformLocation(pass3->shaderProgram, "SCREEN_H"), SCREEN_H);
        pass3->bind_texture("prev_texture", pass1->attach_textures[0]);
    }
    pass3->draw();
    last_frame_tex = pass1->attach_textures[0];
    //--------

    static float tot_dt = 0;
    tot_dt += dt;
    if(tot_dt > 1) {
        tot_dt = 0;
        cout << "FPS: " << 1.0 / dt << endl;
    }

    float speed = 50;
    if(glfwGetKey(window, GLFW_KEY_W)){
        camera->transform.position += camera->transform.direction_z() * -speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_S)){
        camera->transform.position += camera->transform.direction_z() * speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_D)){
        camera->transform.position += camera->transform.direction_x() * speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_A)){
        camera->transform.position += camera->transform.direction_x() * -speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE)) camera->transform.position += vec3(0, speed * dt, 0);
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))  camera->transform.position += vec3(0, -speed * dt, 0);
	if(glfwGetKey(window, GLFW_KEY_Q)) glfwSetWindowShouldClose(window, GL_TRUE);
}

void init() {

    // passes
    pass1 = new Renderer("shader/disney.frag", 3);
    pass2 = new RenderPass("shader/blur.frag", 1);
    pass3 = new RenderPass("shader/direct.frag", 0, true);

    scene = new Scene("Scene");
    camera = new Instance;
    {
        Instance *o1 = Loader::load_model("model/casa_obj.glb");
        o1->transform.rotation = vec3(M_PI / 2, 0, 0);
        scene->add_child(o1);

        Instance *light= Loader::load_model("model/light.obj");
        light->transform.scale = vec3(30, 30, 30);
        light->transform.position = vec3(0, 100, 0);
        light->get_child(0)->meshes[0]->material->emission = vec3(1);
        light->get_child(0)->meshes[0]->material->is_emit = true;
        scene->add_child(light);

//        Instance *light2= Loader::load_model("model/light.obj");
//        light2->transform.scale = vec3(3, 3, 3);
//        light2->transform.position = vec3(-40, 80, 40);
//        light2->get_child(0)->meshes[0]->material->emission = vec3(450);
//        light2->get_child(0)->meshes[0]->material->is_emit = true;
//        scene->add_child(light2);

    }

    skybox = new HDRTexture("img/table_mountain_2_puresky_1k.hdr");

    camera->transform.rotation.y = M_PI;
    camera->transform.position = vec3(1, 1, 1);
    camera->transform.rotation = vec3(0, -M_PI / 3, 0);
    scene->reload();
    pass1->reload_scene(scene);
}

int main(int argc, const char* argv[]) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号
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

    bool show_imgui = true;
    float last_time = glfwGetTime(), detaTime;
    while(!glfwWindowShouldClose(window)) {
        detaTime = glfwGetTime() - last_time;
        last_time += detaTime;
        glfwPollEvents();	//检查有没有发生事件，调用相应回调函数

        update(detaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

//        ImGui::ShowDemoWindow(&show_imgui);
        if (show_imgui) {
            ImGui::Begin("Editor", &show_imgui);
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
