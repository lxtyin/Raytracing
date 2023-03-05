#include <iostream>
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include "src/RenderPass.h"
#include "src/Renderer.h"
#include "src/Tool.h"
#include "src/Config.h"
using namespace std;

GLFWwindow *window;
Renderer *pass1;
RenderPass *pass2, *pass3;
Scene scene;
Object camera;
uint skybox;

void update(float dt) {

    static uint frame = 1, last_frame_tex = 0;

    // 渲染管线
    pass1->use();
    {
        glUniformMatrix4fv(glGetUniformLocation(pass1->shaderProgram, "v2w_mat"), 1, GL_FALSE, glm::value_ptr(camera.transform()));
        glUniform1ui(glGetUniformLocation(pass1->shaderProgram, "frameCounter"), frame++);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_W"), SCREEN_W);
        glUniform1i(glGetUniformLocation(pass1->shaderProgram, "SCREEN_H"), SCREEN_H);
        pass1->bind_texture("last_frame_texture", last_frame_tex);
        pass1->bind_texture("skybox", skybox, GL_TEXTURE_CUBE_MAP);
    }
    uint tex1 = pass1->draw();

    pass2->use();
    {
        glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_W"), SCREEN_W);
        glUniform1i(glGetUniformLocation(pass2->shaderProgram, "SCREEN_H"), SCREEN_H);
        pass2->bind_texture("prev_texture", tex1);
    }
    uint tex2 = pass2->draw();

    pass3->use();
    {
        glUniform1i(glGetUniformLocation(pass3->shaderProgram, "SCREEN_W"), SCREEN_W);
        glUniform1i(glGetUniformLocation(pass3->shaderProgram, "SCREEN_H"), SCREEN_H);
        pass3->bind_texture("prev_texture", tex2);
    }
    pass3->draw();
    last_frame_tex = tex1;
    //--------

    static float tot_dt = 0;
    tot_dt += dt;
    if(tot_dt > 1) {
        tot_dt = 0;
        cout << "FPS: " << 1.0 / dt << endl;
    }

    float speed = 150;
    if(glfwGetKey(window, GLFW_KEY_W)){
        camera.position += camera.direction_z() * -speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_S)){
        camera.position += camera.direction_z() * speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_D)){
        camera.position += camera.direction_x() * speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_A)){
        camera.position += camera.direction_x() * -speed * dt;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE)) camera.position += vec3(0, speed * dt, 0);
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))  camera.position += vec3(0, -speed * dt, 0);
	if(glfwGetKey(window, GLFW_KEY_RIGHT)) camera.rotation += vec3(0, dt, 0);
	if(glfwGetKey(window, GLFW_KEY_LEFT)) camera.rotation += vec3(0, -dt, 0);
	if(glfwGetKey(window, GLFW_KEY_UP)) camera.rotation += vec3(-dt, 0, 0);
	if(glfwGetKey(window, GLFW_KEY_DOWN)) camera.rotation += vec3(dt, 0, 0);
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GL_TRUE);
}

void init() {

    // passes
    pass1 = new Renderer("shader/test.frag");
    pass2 = new RenderPass("shader/blur.frag");
    pass3 = new RenderPass("shader/direct.frag", true);

    // scene;
    {
        Object *o1 = load_obj("model/cornellbox/left.obj")[0];
        o1->material = new Material;
        o1->material->color = vec3(0.8, 0.0, 0.0);
        o1->material->roughness = 0;
        o1->material->metallic = 1;
        scene.objects.push_back(o1);
        Object *o2 = load_obj("model/cornellbox/right.obj")[0];
        o2->material = new Material;
        o2->material->color = vec3(0.14f, 0.45f, 0.091f);
        o2->material->roughness = 0;
        o2->material->metallic = 1;
        scene.objects.push_back(o2);
        Object *o4 = load_obj("model/cornellbox/floor.obj")[0];
        o4->material = new Material;
        o4->material->color = vec3(0.725f, 0.71f, 0.68f);
        scene.objects.push_back(o4);
        Object *o5 = load_obj("model/cornellbox/tallbox.obj")[0];
        o5->material = new Material;
        o5->material->color = vec3(0.725f, 0.71f, 0.68f);
        o5->material->roughness = 0;
        o5->material->metallic = 1;
        scene.objects.push_back(o5);
        Object *o6 = load_obj("model/cornellbox/shortbox.obj")[0];
        o6->material = new Material;
        o6->material->color = vec3(1, 0.2f, 0.68f);
        scene.objects.push_back(o6);
        Object *o3 = load_obj("model/cornellbox/light.obj")[0];
        o3->material = new Material;
        o3->material->is_emit = true;
        o3->material->emission =(8.0f * vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * vec3(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *vec3(0.737f+0.642f,0.737f+0.159f,0.737f));
        scene.objects.push_back(o3);
    }

    skybox = load_cubebox("img/sky_px.png", "img/sky_nx.png", "img/sky_py.png",
                          "img/sky_ny.png", "img/sky_pz.png", "img/sky_nz.png");

    camera.position = vec3(300, 300, -800);
    camera.rotation.y = M_PI;
    scene.reload();
    pass1->reload_scene(&scene);
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
