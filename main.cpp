#include <iostream>
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/RenderPass.h"
#include "src/Renderer.h"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/tool/loader.h"
#include "src/texture/CubeBox.h"
#include "src/texture/Texture.h"
#include "src/texture/HDRTexture.h"
using namespace std;

GLFWwindow *window;
Renderer *pass1;
RenderPass *pass2, *pass3;
Scene *scene;
Instance *camera;
HDRTexture* skybox;

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
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "fast_shade"), false);
        } else {
            frame = 1;
            glUniform1i(glGetUniformLocation(pass1->shaderProgram, "fast_shade"), true);
        }

        pass1->bind_texture("last_frame_texture", last_frame_tex);
        pass1->bind_texture("skybox", skybox->TTO);
        pass1->bind_texture("skybox_samplecache", skybox->sample_cache_tto);
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
	if(glfwGetKey(window, GLFW_KEY_RIGHT)) camera->transform.rotation += vec3(0, dt, 0);
	if(glfwGetKey(window, GLFW_KEY_LEFT)) camera->transform.rotation += vec3(0, -dt, 0);
	if(glfwGetKey(window, GLFW_KEY_UP)) camera->transform.rotation += vec3(-dt, 0, 0);
	if(glfwGetKey(window, GLFW_KEY_DOWN)) camera->transform.rotation += vec3(dt, 0, 0);
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);
}

void init() {

    // passes
    pass1 = new Renderer("shader/disney_old.frag", 3);
    pass2 = new RenderPass("shader/blur.frag", 1);
    pass3 = new RenderPass("shader/direct.frag", 0, true);

    scene = new Scene;
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
    //设置各种选项值
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心渲染模式

    //创建窗口，放入上下文中
    window = glfwCreateWindow(SCREEN_W, SCREEN_H, "My Window", NULL, NULL);
    glfwMakeContextCurrent(window);

    //openGL本质是一个巨大的状态机，很多内容都是通过设置来完成的
    //注册回调函数
    //glfwSetCursorPosCallback(window, mouse_callback);
    //设置鼠标模式
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //初始化glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glDisable(GL_DEPTH_TEST);

    init();

    float last_time = glfwGetTime(), detaTime;
    while(!glfwWindowShouldClose(window)) {
        detaTime = glfwGetTime() - last_time;
        last_time += detaTime;
        update(detaTime);

        glfwSwapBuffers(window); //交换两层颜色缓冲
        glfwPollEvents();	//检查有没有发生事件，调用相应回调函数
    }

    //释放所有资源
    glfwTerminate();
    return 0;
}
