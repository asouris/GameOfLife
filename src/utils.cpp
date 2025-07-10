

#include <random>
#include <iostream>
#include "utils.h"






int worldIdx(int i, int j, int k, const int N, const int M, const int D){
    k = (k + D) % D;
    i = (i + N) % N;
    j = (j + M) % M;
	return k * N * M + i * M + j;
}




/** Resolves mouse button input when called
 *  Clicking inside the simulation will change the state of the clicked square
 */
// void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
// {
//     if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !STYLE){
//         double xpos, ypos;
//         glfwGetCursorPos(window, &xpos, &ypos);

//         double min_simulation = (WIDTH - sim_height) / 2, max_simulation = WIDTH - min_simulation;
//         if(min_simulation <= xpos and xpos < max_simulation and min_simulation <= ypos and ypos < max_simulation){
//             int cell_i = (ypos - min_simulation) / cell_size, cell_j = (xpos - min_simulation) / cell_size;
//             next_state_3d[cell_i + cell_j * cols] = !next_state_3d[cell_i + cell_j * cols];
//         }
//     }
// }


Controller::Controller(int r, int c, int p){
    rows = r, cols = c, planes = p;
    next_state.resize(rows * cols, planes);
    q_3d = initConway(rows, cols, planes, 0, next_state);
}

void Controller::add_n_random_glider(int n){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, next_state.size());
    for (int z=0; z< n; z++){
        int random_point = distr(gen);

        int k = random_point / (rows * cols);  
        int i = (random_point % (rows * cols)) / cols;
        int j = (random_point % (rows * cols)) % cols;

        next_state[random_point] = 1;
        next_state[worldIdx(i+1, j, k, rows, cols, planes)] = 1;
        next_state[worldIdx(i+1, j, k+1, rows, cols, planes)] = 1;
        next_state[worldIdx(i, j, k+1, rows, cols, planes)] = 1;

        next_state[worldIdx(i-1, j-1, k, rows, cols, planes)] = 1;
        next_state[worldIdx(i-1, j-1, k+1, rows, cols, planes)] = 1;

        next_state[worldIdx(i+2, j-1, k, rows, cols, planes)] = 1;
        next_state[worldIdx(i+2, j-1, k+1, rows, cols, planes)] = 1;

        next_state[worldIdx(i, j-1, k+2, rows, cols, planes)] = 1;
        next_state[worldIdx(i+1, j-1, k+2, rows, cols, planes)] = 1;

    }
}

void Controller::kill_world(){
    std::fill(next_state.begin(), next_state.end(), 0);
}



void Camera::update(){
    if (keys[0]) phi-=1;
    if (keys[1]) phi+=1;
    if (keys[2]) theta-=1;
    if (keys[3]) theta+=1;

    if (theta > 90) theta = 90;
    else if(theta < 0) theta = 0.0001;

    position.x = distance * sin(glm::radians(theta)) * sin(glm::radians(phi));
    position.y = distance * cos(glm::radians(theta));
    position.z = distance * sin(glm::radians(theta)) * cos(glm::radians(phi));
}

glm::mat4 Camera::get_camera_view(){
    return glm::lookAt(position, focus, glm::vec3(0.0f, 1.0f, 0.0f));
}







Window::Window(int WIDTH, int HEIGHT, Controller &c){
    init_glfw_window(WIDTH, HEIGHT, "Conway's Game of Life");
    controller = &c;
    //window = glfwWindow;
    //glfwInit();

    //window = glfwCreateWindow(WIDTH, HEIGHT, "Conway's Game of Life", NULL, NULL);        

    // needed for glfwGetUserPointer to work
    glfwSetWindowUserPointer(m_glfwWindow, this);

    // set our static functions as callbacks
    glfwSetKeyCallback(m_glfwWindow, WindowKeyCallback);

    glViewport(0, 0, WIDTH, HEIGHT);
}


void Window::init_glfw_window(int width, int height, const char *title){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    m_glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    if (m_glfwWindow == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(0);
    }

    glfwMakeContextCurrent(m_glfwWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(0);
    }  

    //glfwSetKeyCallback(window, key_callback);
    //glfwSetMouseButtonCallback(window, mouse_button_callback);
}

    /* GLFW CONFIG FUNCTIONS */

/** Resolves key input when called
 *  Space will change the state of the simulation between pause and running
 *  Up and Down will change the fps of the simulation
 */
void Window::internal_key_callback(int key, int action){
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        /*update buffer with possible chanegs*/
        if(!(controller->style_3d)){
            controller->q_3d.updateBuffer(controller->next_state, 0);
        }
        controller->running = !controller->running;
    }

    if(key == GLFW_KEY_UP && action == GLFW_PRESS && controller->current_fps < 60) controller->current_fps += 1;
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS && controller->current_fps > 1) controller->current_fps -= 1;

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
        controller->style_3d = (controller->style_3d + 1) % 2;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
        controller->style_3d = (controller->style_3d + 1) % 2;
    }

    if(key == GLFW_KEY_M && action == GLFW_PRESS){
        controller->add_n_random_glider(5);
    }

    if(key == GLFW_KEY_K && action == GLFW_PRESS){
        controller->kill_world();
    }

    if (key == GLFW_KEY_A){
        if(action == GLFW_PRESS) controller->camera->keys[0] = 1;
        else if(action == GLFW_RELEASE) controller->camera->keys[0] = 0;
    }
    if (key == GLFW_KEY_D){
        if(action == GLFW_PRESS) controller->camera->keys[1] = 1;
        else if(action == GLFW_RELEASE) controller->camera->keys[1] = 0;
    }
    if (key == GLFW_KEY_S){
        if(action == GLFW_PRESS) controller->camera->keys[2] = 1;
        else if(action == GLFW_RELEASE) controller->camera->keys[2] = 0;
    }
    if (key == GLFW_KEY_W){
        if(action == GLFW_PRESS) controller->camera->keys[3] = 1;
        else if(action == GLFW_RELEASE) controller->camera->keys[3] = 0;
    }
}



// Window makeWindow(int WIDTH, int HEIGHT, const char *title, Controller c){
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
//     GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, title, NULL, NULL);
//     if (window == NULL)
//     {
//         std::cout << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         exit(0);
//     }

//     glfwMakeContextCurrent(window);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         exit(0);
//     }  

//     Window program_window = Window(window, WIDTH, HEIGHT, c);
//     glfwSetWindowUserPointer(window, &program_window);

//     glfwSetKeyCallback(window,program_window.key_callback);
//     glfwSetMouseButtonCallback(window, mouse_button_callback);
//     glViewport(0, 0, WIDTH, HEIGHT);


//     return program_window;
// }