#include <bits/stdc++.h>
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "src/Renderer.h"
#include "src/Tool.h"
using namespace std;

#define SCREEN_W 512
#define SCREEN_H 512

GLFWwindow *window;
Renderer renderer;
Scene scene;
Object camera;

void update(float dt) {

    if(false) {
        // 场景更新时
        scene.reload();
        renderer.reload_scene(&scene);
    }
    renderer.draw(&scene, camera.transform());

    static float tot_dt = 0;
    tot_dt += dt;
    if(tot_dt > 1) {
        tot_dt = 0;
        cout << "FPS: " << 1.0 / dt << endl;
    }

    float speed = 50;
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

    // add object;
    Object *o1 = load_obj("model/cornellbox/left.obj");
    o1->material = new Material;
    o1->material->color = vec3(0.63f, 0.065f, 0.05f);
    scene.objects.push_back(o1);
    Object *o2 = load_obj("model/cornellbox/right.obj");
    o2->material = new Material;
    o2->material->color = vec3(0.14f, 0.45f, 0.091f);
    scene.objects.push_back(o2);
    Object *o4 = load_obj("model/cornellbox/floor.obj");
    o4->material = new Material;
    o4->material->color = vec3(0.725f, 0.71f, 0.68f);
    scene.objects.push_back(o4);
    Object *o5 = load_obj("model/cornellbox/tallbox.obj");
    o5->material = new Material;
    o5->material->color = vec3(0.725f, 0.71f, 0.68f);
    scene.objects.push_back(o5);
    Object *o6 = load_obj("model/cornellbox/shortbox.obj");
    o6->material = new Material;
    o6->material->color = vec3(0.725f, 0.71f, 0.68f);
    scene.objects.push_back(o6);
    Object *o3 = load_obj("model/cornellbox/light.obj");
    o3->material = new Material;
    o3->material->is_emit = true;
    o3->material->emission = (8.0f * vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * vec3(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *vec3(0.737f+0.642f,0.737f+0.159f,0.737f));
	o3->position = vec3(100, 0, 100);
    scene.objects.push_back(o3);

    camera.position = vec3(300, 300, -400);
    camera.rotation.y = M_PI;

    scene.reload();
    renderer.init();
    renderer.set_screen(SCREEN_W, SCREEN_H);
    renderer.reload_scene(&scene);
}

//初始化窗口（采用默认设置）
GLFWwindow* initWindow() {
    glfwInit();
    //设置各种选项值
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心渲染模式

    //创建窗口，放入上下文中
    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H, "My Window", NULL, NULL);
    glfwMakeContextCurrent(window);

    //openGL本质是一个巨大的状态机，很多内容都是通过设置来完成的
    //注册回调函数
    //glfwSetCursorPosCallback(window, mouse_callback);
    //设置鼠标模式
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

int main(int argc, const char* argv[]) {

    window = initWindow();
    //初始化glad
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    init();

    float last_time = glfwGetTime(), detaTime;
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

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
