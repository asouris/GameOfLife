

#include <ostream>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"


#include "opencl_conway.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



struct Camera {

    glm::vec3 position = glm::vec3(3.0f, 0.0f, 0.0f);
    glm::vec3 focus =    glm::vec3(0.0f, 0.0f, 0.0f);
    double distance = 3;
    double theta = 90;
    double phi = 0;

    int keys[4] = {0, 0, 0, 0};

    void update();
    glm::mat4 get_camera_view();

};

struct Controller{
    /* simulation constants */
    int WIDTH, HEIGHT;
    int rows, cols, planes;
    float SIM_SCALE = 0.8;
    float CELL_SIZE = 10.0;
    float cell_gl_size;

    /* simulation state variables */
    int current_fps = 10;   /* simulation fps, used as simulation velocity */
    bool running = 0;       /* if True, the simulation is running, otherwise is paused */
    int style_3d = 0;       /* if True a 3d style is used, its 2d*/

    /* openCL variables */
    std::vector <int> next_state; /* holds the next state in simulation, updated by OpenCL */
    Queue q_3d;

    /* openGL uniforms */
    float cell_color[4] = {1, 1, 1, 1};
    float sun_intensity = 1;

    std::vector<float> lighted_cells_positions;
    int number_of_light_cells = 0;
    int internal_number_of_light_cells = 0;
    float light_cells_intensity = 1;

    int coloring_style = 0;

    Camera camera = Camera();
    

    Controller(int WIDTH, int HEIGHT);
    void add_n_random_glider(int n);
    void kill_world();


    unsigned int load_shader(std::string path, bool shader_type);
    unsigned int create_shader_program(unsigned int vertex, unsigned int fragment);


    int update_with_step(unsigned int &positions, std::vector<int> &result, std::vector<float> coords, int N, int M, int D);
    void bind_load_static_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index);
    void bind_load_normals_buffer(unsigned int *VBOs, unsigned int *VAOs, int size, float *data, int index);
    void bind_load_indices_buffer(unsigned int *VBOs, unsigned int *VAOs, unsigned int EBO, int size, float *data, int indices_size, int *indices_data, int index);


    std::vector<float> gridLines();
    std::vector<float> grid_points_3d();

    void fill_lighted_cells(std::vector<float> &points);
    void update_light_cells(std::vector<float> &points);

    void renderImgui(GLFWwindow* window, ImGuiIO &io);
};

class Window {
public:
    GLFWwindow *m_glfwWindow;
    ImGuiIO *io;

    Window(Controller &c);

    auto internal_key_callback(int key, int action)-> void;
    auto internal_mouse_button_callback(int button, int action, int mods)-> void;


private:
    Controller *controller;
    int WIDTH, HEIGHT;

    void init_glfw_window(int width, int height, const char *title);

    inline static auto WindowKeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods)  -> void {
        
            Window *window = static_cast<Window*>(glfwGetWindowUserPointer(win));
            window->internal_key_callback(key, action);
    }

    inline static auto WindowMouseButtonCallback(GLFWwindow* win, int button, int action, int mods)  -> void {
        
            Window *window = static_cast<Window*>(glfwGetWindowUserPointer(win));
            window->internal_mouse_button_callback(button, action, mods);
    }
    
};

