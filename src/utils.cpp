

#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include "utils.h"

/** Returns 1d coordinates from 3d coordinates
 * @param i x coordinate
 * @param j y coordinate
 * @param k z coordiante
 * @param N number of rows
 * @param M number of columns
 * @param D number of planes
 */
int worldIdx(int i, int j, int k, const int N, const int M, const int D){
    k = (k + D) % D;
    i = (i + N) % N;
    j = (j + M) % M;
	return k * N * M + i * M + j;
}

Controller::Controller(int width, int height){
    rows = width * SIM_SCALE / CELL_SIZE, cols = height * SIM_SCALE / CELL_SIZE, planes = rows;
    WIDTH = width, HEIGHT = height;

    next_state.resize(rows * cols * planes);
    q_3d = initConway(rows, cols, planes, 0, next_state);
    cell_gl_size = 2.0f * SIM_SCALE / (float)rows;
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

        if(style_3d){
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
        else{
            next_state[worldIdx(i, j, 0, rows, cols, planes)] = 1;
            next_state[worldIdx(i+1, j+1, 0, rows, cols, planes)] = 1;
            next_state[worldIdx(i+2, j+1, 0, rows, cols, planes)] = 1;
            next_state[worldIdx(i+2, j, 0, rows, cols, planes)] = 1;
            next_state[worldIdx(i+2, j-1, 0, rows, cols, planes)] = 1;
        }

    }
}

void Controller::kill_world(){
    std::fill(next_state.begin(), next_state.end(), 0);
}

unsigned int Controller::load_shader(std::string path, bool shader_type){
    /*reading shader*/
    std::ifstream shaderInput;
    shaderInput.open(path);
    std::stringstream strStream;
    strStream << shaderInput.rdbuf() << "\n\0"; //read the file
    const std::string& tmp = strStream.str();   
    const char *shader_source = tmp.c_str();

    shaderInput.close();

    /*loading shader into opengl*/
    int success;
    char infoLog[512];

    unsigned int shader;
    if(shader_type) shader = glCreateShader(GL_FRAGMENT_SHADER);
    else            shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

unsigned int Controller::create_shader_program(unsigned int vertex, unsigned int fragment){
    int success;
    char infoLog[512];
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    // check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return program;
}

int Controller::update_with_step(unsigned int &positions, std::vector<int> &result, std::vector<float> coords, int N, int M, int D){
    int number_of_active_cells = 0;

    std::vector <float> new_positions;
    for(int i = 0; i < N*M*D; i++){
        if(result[i]){
            new_positions.insert(new_positions.end(), {coords[i*3], coords[i*3 + 1], coords[i*3 + 2]});
            number_of_active_cells+=1;
        }
    }


    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*new_positions.size(), new_positions.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return number_of_active_cells;

}

void Controller::bind_load_static_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index){
    glBindVertexArray(VAOs[index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[index]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void Controller::bind_load_normals_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index){
    glBindVertexArray(VAOs[index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[index]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void Controller::bind_load_indices_buffer(unsigned int *VBOs, unsigned int *VAOs, unsigned int EBO, int size, float *data, int indices_size, int *indices_data, int index){
    glBindVertexArray(VAOs[index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[index]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

std::vector<float> Controller::gridLines(){
    std::vector <float> vertices;
    int i = 0;
    while(i <= rows){
        vertices.insert(vertices.end(), {           //colors
            cell_gl_size*i - 0.8f, 0.8f, 0.0f,      0.8f, 0.8f, 0.8f,
            cell_gl_size*i - 0.8f, -0.8f, 0.0f,     0.8f, 0.8f, 0.8f });
        i++;
    }
    i = 0;
    while(i <= cols){
        vertices.insert(vertices.end(), {           //colors
            0.8f, cell_gl_size * i - 0.8f, 0.0f,    0.8f, 0.8f, 0.8f,
            -0.8f, cell_gl_size*i - 0.8f, 0.0f,     0.8f, 0.8f, 0.8f});
        i++;
    }
    return vertices;
}

std::vector<float> Controller::grid_points_3d(){
    std::vector<float> vertices;
    for(int k = planes-1; k >= 0; k--){
        for(int i = 0 ; i < rows; i++){
            for(int j = cols-1; j >= 0; j--){
                vertices.insert(vertices.end(), {cell_gl_size*(float)i - 0.8f, cell_gl_size*(float)j - 0.8f, cell_gl_size*(float)k - 0.8f});
            }
        }
    }

    return vertices;
}

void Controller::fill_lighted_cells(std::vector<float> &points){
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distr(0, points.size()/3);
    while(lighted_cells_positions.size()/3 < internal_number_of_light_cells){
        int random_point = distr(gen);
        lighted_cells_positions.insert(lighted_cells_positions.end(), {points[random_point*3], points[random_point*3 + 1], points[random_point*3 + 2]});
    }
}

void Controller::update_light_cells(std::vector<float> &points){
    if(number_of_light_cells > internal_number_of_light_cells){
        internal_number_of_light_cells = number_of_light_cells;
        fill_lighted_cells(points);
    }
    else if(number_of_light_cells < internal_number_of_light_cells){
        lighted_cells_positions.resize(number_of_light_cells * 3);
        internal_number_of_light_cells = number_of_light_cells;
    }
}

void Controller::renderImgui(GLFWwindow* window, ImGuiIO &io){

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    /*Contents of the window*/
    {
        ImGui::Begin("Controller");    
        ImGui::SetWindowPos(ImVec2(WIDTH*0.6, 20));                     

        ImGui::SliderInt("Velocity", &current_fps, 0, 60);  
        
        ImGui::ColorEdit4("Color", cell_color);    
        
        ImGui::SliderFloat("Light Intensity", &sun_intensity, 0, 1);

        const char* items[] = { "2D", "3D" };
        ImGui::Combo("Dimensions", &style_3d, items, IM_ARRAYSIZE(items));

        const char* items2[] = { "Phong", "Normal" };
        ImGui::Combo("Coloring", &coloring_style, items2, IM_ARRAYSIZE(items2));

        const char* items3[] = {"Sequential", "Parallel"};
        ImGui::Combo("Type of simulation", &parallel_simualtion, items3, IM_ARRAYSIZE(items3));

        ImGui::SliderInt("Light Cells", &number_of_light_cells, 0, 20);
        ImGui::SliderFloat("Brightness", &light_cells_intensity, 0, 1);


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }


    /*Actual rendering*/
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
}

void Controller::calculateStepSecuentially(){

    std::vector<int> temp(next_state.size());
    
    for(int gindex = 0; gindex < next_state.size(); gindex++){
        int k = gindex / (rows * cols);  
        int i = (gindex % (rows * cols)) / cols;
        int j = (gindex % (rows * cols)) % cols;
        int neighbours = next_state[worldIdx(i - 1, j - 1, k, rows, cols, planes)] + next_state[worldIdx(i - 1, j, k, rows, cols, planes)] + next_state[worldIdx(i - 1, j + 1, k, rows, cols, planes)] + // same k
                        next_state[worldIdx(i, j - 1, k, rows, cols, planes)] + next_state[worldIdx(i, j + 1, k, rows, cols, planes)] +
                        next_state[worldIdx(i + 1, j - 1, k, rows, cols, planes)] + next_state[worldIdx(i + 1, j, k, rows, cols, planes)] + next_state[worldIdx(i + 1, j + 1, k, rows, cols, planes)];

        if(style_3d){
            neighbours +=   next_state[worldIdx(i - 1, j - 1, k+1, rows, cols, planes)] + next_state[worldIdx(i - 1, j, k+1, rows, cols, planes)] + next_state[worldIdx(i - 1, j + 1, k+1, rows, cols, planes)] + // next k
                            next_state[worldIdx(i, j - 1, k+1, rows, cols, planes)] + next_state[worldIdx(i, j + 1, k+1, rows, cols, planes)] + next_state[worldIdx(i, j, k+1, rows, cols, planes)] +
                            next_state[worldIdx(i + 1, j - 1, k+1, rows, cols, planes)] + next_state[worldIdx(i + 1, j, k+1, rows, cols, planes)] + next_state[worldIdx(i + 1, j + 1, k+1, rows, cols, planes)] +
                            next_state[worldIdx(i - 1, j - 1, k-1, rows, cols, planes)] + next_state[worldIdx(i - 1, j, k-1, rows, cols, planes)] + next_state[worldIdx(i - 1, j + 1, k-1, rows, cols, planes)] + // last k
                            next_state[worldIdx(i, j - 1, k-1, rows, cols, planes)] + next_state[worldIdx(i, j + 1, k-1, rows, cols, planes)] + next_state[worldIdx(i, j, k-1, rows, cols, planes)] +
                            next_state[worldIdx(i + 1, j - 1, k-1, rows, cols, planes)] + next_state[worldIdx(i + 1, j, k-1, rows, cols, planes)] + next_state[worldIdx(i + 1, j + 1, k-1, rows, cols, planes)];
            temp[gindex] = next_state[gindex] && (4 <= neighbours && neighbours <= 5) || !next_state[gindex] && neighbours == 5;
        }
        else{
            temp[gindex] = neighbours == 3 || (neighbours == 2 && next_state[gindex]);        
        }
    }

    std::copy(temp.begin(), temp.end(), next_state.begin());   
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

Window::Window(Controller &c){

    WIDTH = c.WIDTH, HEIGHT = c.HEIGHT;

    init_glfw_window(WIDTH, HEIGHT, "Conway's Game of Life");

    controller = &c;
    glfwSetWindowUserPointer(m_glfwWindow, this);

    glfwSetKeyCallback(m_glfwWindow, WindowKeyCallback);
    glfwSetMouseButtonCallback(m_glfwWindow, WindowMouseButtonCallback);
    glfwSetScrollCallback(m_glfwWindow, WindowScrollCallback);

    /*ImGui setup*/
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    (void)io;
    (*io).ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    (*io).ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    

    /*OpenGL config*/
    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
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
}

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
        if(action == GLFW_PRESS) controller->camera.keys[0] = 1;
        else if(action == GLFW_RELEASE) controller->camera.keys[0] = 0;
    }
    if (key == GLFW_KEY_D){
        if(action == GLFW_PRESS) controller->camera.keys[1] = 1;
        else if(action == GLFW_RELEASE) controller->camera.keys[1] = 0;
    }
    if (key == GLFW_KEY_S){
        if(action == GLFW_PRESS) controller->camera.keys[2] = 1;
        else if(action == GLFW_RELEASE) controller->camera.keys[2] = 0;
    }
    if (key == GLFW_KEY_W){
        if(action == GLFW_PRESS) controller->camera.keys[3] = 1;
        else if(action == GLFW_RELEASE) controller->camera.keys[3] = 0;
    }
}

void Window::internal_mouse_button_callback(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !controller->style_3d){
        double xpos, ypos;
        glfwGetCursorPos(m_glfwWindow, &xpos, &ypos);

        double min_simulation = (WIDTH - (WIDTH * controller->SIM_SCALE)) / 2, max_simulation = WIDTH - min_simulation;
        if(min_simulation <= xpos and xpos < max_simulation and min_simulation <= ypos and ypos < max_simulation){

            int cell_i = (ypos - min_simulation) / controller->CELL_SIZE, cell_j = (xpos - min_simulation) / controller->CELL_SIZE;

            controller->next_state[cell_i + cell_j * controller->cols] = !controller->next_state[cell_i + cell_j * controller->cols];
        }
    }
}

void Window::internal_scroll_callback(double xoffset, double yoffset){

    controller->camera.distance += yoffset;
}