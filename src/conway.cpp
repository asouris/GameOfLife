#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

#include <opencl_conway.h>

/*GLOBAL CONSTANTS*/
const int WIDTH = 1000, HEIGHT = 1000;
const int sim_width = 800, sim_height = 800;
const int cell_size = 10;
const int rows = sim_height/cell_size, cols = sim_width/cell_size;

const float window_fraction = (float)sim_width / (float)WIDTH, usable_gl_space = 2.0f * window_fraction;
const float cell_gl_size = usable_gl_space / (float)rows;

const int SIM_TYPE = 0;

/*GLOBAL VARIABLES*/
int planes = rows;
float MAX_FPS = 10.f;                                   /*variable fps for rendering*/
bool STATE = 0;                                         /*simulation state, 0 : pause, 1 : running*/
int STYLE = 0;                                          /*simulation style, 0 : 2D, 1 : 2D with lights, 2 : 3D simple*/
std::vector <int> next_state_2d(rows * cols);           /*holds next state on simulation*/
std::vector <int> next_state_3d(rows * cols * planes);  /*holds next state on 3d simulation*/
//Queue q = initConway(rows, cols, SIM_TYPE, next_state_2d, planes);      /*opencl command queue*/
Queue q_3d = initConway(rows, cols, SIM_TYPE, next_state_3d, planes);   /*opencl command queue for 3d simulation*/

//Queue *current_queue = &q;
std::vector <int> *current_next_state = &next_state_2d;

std::vector<float> lighted_cells_2d;
int number_lighted_cells_2d = 2;

int worldIdx(int i, int j, int k, const int N, const int M, const int D){
    k = (k + D) % D;
    i = (i + N) % N;
    j = (j + M) % M;
	return k * N * M + i * M + j;
}

void add_n_alive_cells(int n){
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distr(0, next_state_3d.size());
    for(int z = 0; z < n; z++){
        int random_point = distr(gen);

        int k = random_point / (rows * cols);  
        int i = (random_point % (rows * cols)) / cols;
        int j = (random_point % (rows * cols)) % cols;
        
        next_state_3d[random_point] = 1;
        next_state_3d[worldIdx(i, j, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j-1, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j-1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j-1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j-1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j-1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j-1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j-1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j-1, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j-1, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j+1, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j+1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i, j+1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j+1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j+1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j+1, k+1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j+1, k-1, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i-1, j+1, k, rows, cols, planes)] = 1;
        next_state_3d[worldIdx(i+1, j+1, k, rows, cols, planes)] = 1;
    }
}

void kill_world(){
    std::fill(next_state_3d.begin(), next_state_3d.end(), 0);
}

/* GLFW CONFIG FUNCTIONS */

/** Resizes viewport when called */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 

/** Resolves key input when called
 *  Space will change the state of the simulation between pause and running
 *  Up and Down will change the fps of the simulation
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        /*update buffer with possible chanegs*/
        if(!STATE){
            q_3d.updateBuffer(next_state_3d, 0);
        }
        STATE = !STATE;
    }

    if(key == GLFW_KEY_UP && action == GLFW_PRESS && MAX_FPS < 60) MAX_FPS += 1;
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS && MAX_FPS > 1) MAX_FPS -= 1;

    if (key == GLFW_KEY_S && action == GLFW_PRESS){
        // if(STYLE + 1 == 2){
        //     current_queue = &q_3d;
        //     current_next_state = &next_state_3d;
        //     planes = rows;
        // }
        // else{
        //     current_queue = &q;
        //     current_next_state = &next_state_2d;
        //     planes = 1;
        // }
        STYLE = (STYLE + 1) % 3;
    }

    if(key == GLFW_KEY_M && action == GLFW_PRESS){
        add_n_alive_cells(10);
    }

    if(key == GLFW_KEY_K && action == GLFW_PRESS){
        kill_world();
    }
}

/** Resolves mouse button input when called
 *  Clicking inside the simulation will change the state of the clicked square
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        double min_simulation = (WIDTH - sim_height) / 2, max_simulation = WIDTH - min_simulation;
        if(min_simulation <= xpos and xpos < max_simulation and min_simulation <= ypos and ypos < max_simulation){
            int cell_i = (ypos - min_simulation) / cell_size, cell_j = (xpos - min_simulation) / cell_size;
            next_state_3d[cell_i + cell_j * cols] = !next_state_3d[cell_i + cell_j * cols];
        }
    }
}

/** Initializes the glfw window
 *  @return a pointer to the window object
 */
GLFWwindow * init_glfw_window(int width, int height, const char *title){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(0);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(0);
    }  

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glViewport(0, 0, width, height);

    return window;
}

/* SHADERS FUNCTIONS */

/** Loads a shader from a path.
 * @param path path to shader
 * @return shader identifier
*/
unsigned int load_shader(std::string path, bool shader_type){
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

/** Creates shader program from shaders, if said so, it deletes the used shaders afterwards.
 * @param vertex vertex shader already loaded
 * @param fragment fragment shader already loaded
 * @param flag if true deletes the shaders after using them.
 * @return shader program identifier
 */
unsigned int create_shader_program(unsigned int vertex, unsigned int fragment){
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

/* BUFFER FUNCTIONS */

/** Gets the number of active cells on the result array and updates the buffer
 * @param positions buffer to be updated with the positions of active cells
 * @param result    array with the result from a Conway step 
 * @param coords    array with coordinates for every cube
 * @param N         amount of rows
 * @param M         amount of columns
 * @return          number of active cells after step
 */
int update_with_step(unsigned int &positions, std::vector<int> &result, std::vector<float> coords, int N, int M, int D){
    int number_of_active_cells = 0;

    std::vector <float> new_positions;
    for(int i = 0; i < N*M*D; i++){
        if(result[i]){
            new_positions.insert(new_positions.end(), {coords[i*3], coords[i*3 + 1], coords[i*3 + 2]});
            number_of_active_cells+=1;
        }
    }

    std::cout << new_positions.size() << "\n";


    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*new_positions.size(), new_positions.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // glEnableVertexAttribArray(2);
    // glBindBuffer(GL_ARRAY_BUFFER, positions);
    // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);	
    // glVertexAttribDivisor(2, 1); 

    return number_of_active_cells;

}

/** Binds and loads a static buffer of floats
 * @param VBOS array of vertex buffer objects
 * @param VAOs array of vertex array objects
 * @param size size of data array in bytes
 * @param data data array 
 * @param index index of the buffer
*/
void bind_load_static_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index){
    glBindVertexArray(VAOs[index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[index]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void bind_load_normals_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index){
    glBindVertexArray(VAOs[index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[index]);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6* sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

void bind_load_indices_buffer(unsigned int *VBOs, unsigned int *VAOs, unsigned int EBO, int size, float *data, int indices_size, int *indices_data, int index){
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
    //glBindVertexArray(0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* VERTICES FUNCTIONS */

/** Calculates vertices for the grid
 * @return float vector with vertices, 6 values per vertex
 */
std::vector<float> gridLines(){
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

/** Calculates positions for each square in openGL space [-1, 1]
 * @return float vector with positions, 3 values per position
*/
std::vector <float> gridPoints(){
    std::vector < float> vertices;
    for(int i = 0; i < rows; i++){
        for(int j = cols-1; j >= 0; j--){
            vertices.insert(vertices.end(), {cell_gl_size*(float)i - 0.8f, cell_gl_size*(float)j - 0.8f, 0.0f});
        }
    }

    return vertices;
}

/** Calculates positions for each cube in openGL space [-1, 1]
 * @return float vector with positions, 3 values per position
*/
std::vector<float> grid_points_3d(){
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



/* LIGHTED CELLS FUNCTIONS*/

void fill_lighted_cells(std::vector<float> &points){
    std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distr(0, points.size()/3);
    while(lighted_cells_2d.size()/3 < number_lighted_cells_2d){
        int random_point = distr(gen);
        lighted_cells_2d.insert(lighted_cells_2d.end(), {points[random_point*3], points[random_point*3 + 1], points[random_point*3 + 2]});
    }
}

void randomize_lighted_cells(std::vector<float> &points){
    lighted_cells_2d.clear();
    fill_lighted_cells(points);
}

void resize_lighted_cells(int n, std::vector<float> &points){
    if(n < number_lighted_cells_2d){
        lighted_cells_2d.resize(n*3);
        number_lighted_cells_2d = n;
    }
    else if(n > number_lighted_cells_2d){
        number_lighted_cells_2d = n;
        fill_lighted_cells(points);
    }

}


/* MAIN */

int main()
{

    /*Window Configuration*/
    GLFWwindow* window = init_glfw_window(WIDTH, HEIGHT, "Conway's Game of Life");

    /*Load and create shader variables*/
    unsigned int vertex_shader = load_shader("shaders/vshader.glsl", 0);
    unsigned int grid_shader = load_shader("shaders/v_grid.glsl", 0);
    unsigned int fragment_shader = load_shader("shaders/fshader.glsl", 1);
    unsigned int vertex_light_shader = load_shader("shaders/v_lighted_shader.glsl", 0);
    unsigned int fragment_light_shader = load_shader("shaders/f_lighted_shader.glsl", 1);
    unsigned int vertex_3d_shader = load_shader("shaders/v_3d_shader.glsl", 0);
    unsigned int fragment_3d_shader = load_shader("shaders/f_3d_shader.glsl", 1);

    /*Define shader programs*/
    unsigned int shader_program = create_shader_program(vertex_shader, fragment_shader);
    unsigned int grid_shader_program = create_shader_program(grid_shader, fragment_shader);
    unsigned int lighted_shader_program = create_shader_program(vertex_light_shader, fragment_light_shader);
    unsigned int lighted_grid_shader_program = create_shader_program(grid_shader, fragment_light_shader);
    unsigned int shader_program_3d = create_shader_program(vertex_3d_shader, fragment_3d_shader);

    /*Delete shaders once they are in a program*/
    glDeleteShader(vertex_shader);
    glDeleteShader(grid_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_light_shader);
    glDeleteShader(fragment_light_shader);
    glDeleteShader(vertex_3d_shader);
    glDeleteShader(fragment_3d_shader);

    /*Get vertices for the gridlines and positions of all squares*/
    std::vector<float> grid = gridLines(), points = gridPoints(), points_3d = grid_points_3d();
    float *grid_lines_vertices = grid.data(), *grid_points_vertices = points.data(), *grid_points_3d_vertices = points_3d.data();
    
    /*Primitive for a white square*/
    float quadVertices[] = {
        // positions                        // colors
        0.0f,  0.0f, 0.0f,                  0.8f, 0.8f, 0.8f,
        cell_gl_size, cell_gl_size, 0.0f,   0.8f, 0.8f, 0.8f,
        0.0f,  cell_gl_size, 0.0f,          0.8f, 0.8f, 0.8f,

        0.0f,  0.0f, 0.0f,                  0.8f, 0.8f, 0.8f,
        cell_gl_size, 0.0f, 0.0f,           0.8f, 0.8f, 0.8f,
        cell_gl_size,  cell_gl_size, 0.0f,  0.8f, 0.8f, 0.8f	
    };

    float cube_vertices[] = {
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   // 0
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   // 1
        0.0, cell_gl_size, 0.0,                   0.8f, 0.8f, 0.8f,   // 2
        0.0, cell_gl_size, cell_gl_size,          0.8f, 0.8f, 0.8f,   // 3
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   // 4
        cell_gl_size, 0.0, cell_gl_size,          0.8f, 0.8f, 0.8f,   // 5
        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   // 6
        cell_gl_size, cell_gl_size, cell_gl_size, 0.8f, 0.8f, 0.8f    // 7
    };

    float new_cube_vertices[] = {                 // colors           normales
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 0
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 1
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 4
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 4
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 0
        cell_gl_size, 0.0, cell_gl_size,          0.8f, 0.8f, 0.8f,   0.0f, -1.0f, 0.0f, // 5

        0.0, cell_gl_size, cell_gl_size,          0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 3
        0.0, cell_gl_size, 0.0,                   0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 2
        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 6
        0.0, cell_gl_size, cell_gl_size,          0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 3
        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 6
        cell_gl_size, cell_gl_size, cell_gl_size, 0.8f, 0.8f, 0.8f,   0.0f, 1.0f, 0.0f,  // 7

        0.0, cell_gl_size, 0.0,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 2
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 0
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 4
        0.0, cell_gl_size, 0.0,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 2
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 4
        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   0.0, 0.0, -1.0, // 6

        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 6
        cell_gl_size, 0.0, 0.0,                   0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 4
        cell_gl_size, 0.0, cell_gl_size,          0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 5
        cell_gl_size, cell_gl_size, 0.0,          0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 6
        cell_gl_size, 0.0, cell_gl_size,          0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 5
        cell_gl_size, cell_gl_size, cell_gl_size, 0.8f, 0.8f, 0.8f,   1.0, 0.0, 0.0,  // 7

        cell_gl_size, cell_gl_size, cell_gl_size, 0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 7
        cell_gl_size, 0.0, cell_gl_size,          0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 5
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 1
        cell_gl_size, cell_gl_size, cell_gl_size, 0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 7
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 1
        0.0, cell_gl_size, cell_gl_size,          0.8f, 0.8f, 0.8f,   0.0, 0.0, 1.0,  // 3

        0.0, cell_gl_size, cell_gl_size,          0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0, // 3
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0, // 1
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0, // 0
        0.0, 0.0, cell_gl_size,                   0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0, // 1
        0.0, 0.0, 0.0,                            0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0, // 0
        0.0, cell_gl_size, 0.0,                   0.8f, 0.8f, 0.8f,   -1.0, 0.0, 0.0  // 2
    };
    

    int cube_indices[] = {
        0, 4, 6, // frente
        0, 6, 2, 
        4, 5, 7, // lado derecho
        4, 5, 6, 
        1, 0, 2, // lado izquierdo
        1, 2, 3,
        2, 6, 7, // arriba
        2, 7, 3,
        5, 1, 3, // atras
        5, 3, 7,
        1, 5, 4, // abajo
        1, 4, 0
    };

    float big_cube[] = {
        -0.8f, -0.8f, -0.8f,                           0.8f, 0.8f, 0.8f,   // 0
        -0.8f, -0.8f, 0.8f,                   0.8f, 0.8f, 0.8f,   // 1
        -0.8f, 0.8f, -0.8f,                   0.8f, 0.8f, 0.8f,   // 2
        -0.8f, 0.8f, 0.8f,          0.8f, 0.8f, 0.8f,   // 3
        0.8f, 0.8f, 0.8f,                   0.8f, 0.8f, 0.8f,   // 4
        0.8f, 0.8f, -0.8f,          0.8f, 0.8f, 0.8f,   // 5
        0.8f, -0.8f, 0.8f,          0.8f, 0.8f, 0.8f,   // 6
        0.8f, -0.8f, -0.8f,  0.8f, 0.8f, 0.8f    // 7
    };

    int big_cube_indice[] = {
        0, 1,
        1, 6, 
        6, 7, 
        7, 0,
        2, 3,
        3, 4,
        4, 5,
        5, 2,
        0, 2,
        7, 5,
        1, 3, 
        6, 4
    };

    /*Vertex Buffer Object and Vertex Array Object creation*/
    unsigned int VBOs[3], VAOs[3], EBO;
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);
    glGenBuffers(1, &EBO);

    /*binding the first array: grid array*/
    bind_load_static_buffer(VBOs, VAOs, sizeof(float)*grid.size(), grid_lines_vertices, 0);

    /*binding second array: just a quad*/
    //bind_load_static_buffer(VBOs, VAOs, sizeof(quadVertices), quadVertices, 1);
    bind_load_normals_buffer(VBOs, VAOs, sizeof(new_cube_vertices), new_cube_vertices, 1);

    /*Binding a special array for instancing the quad, it will hold positions*/
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points_3d.size(), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);	
    glVertexAttribDivisor(2, 1);  

    bind_load_indices_buffer(VBOs, VAOs, EBO, sizeof(big_cube), big_cube, sizeof(big_cube_indice), big_cube_indice, 2);

    randomize_lighted_cells(points);

    glm::mat4 view_3d          = glm::mat4(1.0f);
    glm::mat4 projection_3d    = glm::mat4(1.0f);
    glm::mat4 view             = glm::mat4(1.0f);
    glm::mat4 projection       = glm::mat4(1.0f);

    view_3d  = glm::lookAt(glm::vec3(3.0f, 0.0f, 5.0f), 
                           glm::vec3(0.0f, 0.0f, 0.0f), 
                           glm::vec3(0.0f, 1.0f, 0.0f));
    projection_3d = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        
    
    /*Rendering function*/
    auto render = [&](){
        /*background color*/
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        unsigned int viewLoc  = glGetUniformLocation(grid_shader_program, "view");
        unsigned int projLoc  = glGetUniformLocation(grid_shader_program, "projection");
        glUseProgram(grid_shader_program);
        /*draw background grid*/
        if(STYLE == 2){
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_3d));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_3d));
            glBindVertexArray(VAOs[2]);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        }
        else{
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(VAOs[0]);
            glDrawArrays(GL_LINES, 0, grid.size()/3);
        }

        glBindVertexArray(0);
        

        /*if not in pause*/
        if(STATE){
            /*Calculating conway step*/
            calculateStep(rows, cols, q_3d, next_state_3d, planes, STYLE == 2);
        }
        
        /*updates the positions buffer and gets the number of active cells*/
        int number_of_active_cells = update_with_step(instanceVBO, next_state_3d, points_3d, rows, cols, planes);

        //std::cout << number_of_active_cells << "\n";
        viewLoc  = glGetUniformLocation(shader_program_3d, "view");
        projLoc  = glGetUniformLocation(shader_program_3d, "projection");

        glUseProgram(shader_program_3d);
        if(STYLE == 2){
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_3d));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_3d));
        }
        else{
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }
        
        glBindVertexArray(VAOs[1]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 30, number_of_active_cells);
        //}

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    };


    /*Window loop, controls the speed for the simulation*/
    float last_time = glfwGetTime();
    float last_update_time = glfwGetTime();
    float accumulated_time = 0.f;

    while (!glfwWindowShouldClose(window))
    {
        float current_time = glfwGetTime();
        float dt = current_time - last_time;
        last_time = current_time;
        accumulated_time += dt;

        glfwPollEvents();

        /*rendering*/
        float FRAME_TIME = 1.f / MAX_FPS;
        if (accumulated_time >= FRAME_TIME)
        {
            float update_dt = current_time - last_update_time;
            last_update_time = current_time;
            accumulated_time = 0.f;

            render();
        }
    
    }

    /*if the window was closed, frees memory*/
    glDeleteVertexArrays(4, VAOs);
    glDeleteBuffers(4, VBOs);
    glDeleteProgram(shader_program);
    glDeleteProgram(grid_shader_program);

    glfwTerminate();
    return 0;   
  
}
