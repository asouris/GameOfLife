

#include <ostream>
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
    int rows, cols, planes;

    /* simulation state variables */
    int current_fps = 10;   /* simulation fps, used as simulation velocity */
    bool running = 0;       /* if True, the simulation is running, otherwise is paused */
    bool style_3d = 0;      /* if True a 3d style is used, its 2d*/

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

    Camera *camera;
    

    Controller(int r, int c, int p);
    void add_n_random_glider(int n);
    void kill_world();
};

class Window {
public:
    Window(int WIDTH, int HEIGHT, Controller &c);
    auto internal_key_callback(int key, int action)           -> void;

private:
    GLFWwindow *m_glfwWindow;
    Controller *controller;

    void init_glfw_window(int width, int height, const char *title);

    // Here are our callbacks. I like making them inline so they don't take up
    // any of the cpp file
    inline static auto WindowKeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods)  -> void {
        
            Window *window = static_cast<Window*>(glfwGetWindowUserPointer(win));
            window->internal_key_callback(key, action);
    }
    
};
