#include "utils.h"

// #include "imgui.h"
// #include "backends/imgui_impl_glfw.h"
// #include "backends/imgui_impl_opengl3.h"

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <math.h>

const int WIDTH = 1000, HEIGHT = 1000;

/* MAIN */

int main()
{

    /* Controller */
    Controller controller = Controller(WIDTH, HEIGHT);

    /* Window */
    Window window = Window(controller);

    /*Load and create shader variables*/
    unsigned int grid_vertex = controller.load_shader("shaders/grid_vertex.glsl", 0);
    unsigned int grid_fragment = controller.load_shader("shaders/grid_fragment.glsl", 1);
    unsigned int vertex_3d_shader = controller.load_shader("shaders/3d_vertex.glsl", 0);
    unsigned int fragment_3d_shader = controller.load_shader("shaders/3d_fragment.glsl", 1);

    /*Define shader programs*/
    unsigned int grid_shader_program = controller.create_shader_program(grid_vertex, grid_fragment);
    unsigned int shader_program_3d = controller.create_shader_program(vertex_3d_shader, fragment_3d_shader);

    /*Delete shaders once they are in a program*/
    glDeleteShader(grid_vertex);
    glDeleteShader(grid_fragment);
    glDeleteShader(vertex_3d_shader);
    glDeleteShader(fragment_3d_shader);

    /*Get vertices for the gridlines and positions of all squares*/
    std::vector<float> grid = controller.gridLines(), points_3d = controller.grid_points_3d();
    float *grid_lines_vertices = grid.data(),*grid_points_3d_vertices = points_3d.data();
    
    /*Primitive for a white cube*/
    float new_cube_vertices[] = {                                                   //normales
        0.0, 0.0, 0.0,                                                              0.0f, -1.0f, 0.0f, // 0
        0.0, 0.0, controller.cell_gl_size,                                          0.0f, -1.0f, 0.0f, // 1
        controller.cell_gl_size, 0.0, 0.0,                                          0.0f, -1.0f, 0.0f, // 4
        controller.cell_gl_size, 0.0, 0.0,                                          0.0f, -1.0f, 0.0f, // 4
        0.0, 0.0, 0.0,                                                              0.0f, -1.0f, 0.0f, // 0
        controller.cell_gl_size, 0.0, controller.cell_gl_size,                      0.0f, -1.0f, 0.0f, // 5

        0.0, controller.cell_gl_size, controller.cell_gl_size,                      0.0f, 1.0f, 0.0f,  // 3
        0.0, controller.cell_gl_size, 0.0,                                          0.0f, 1.0f, 0.0f,  // 2
        controller.cell_gl_size, controller.cell_gl_size, 0.0,                      0.0f, 1.0f, 0.0f,  // 6
        0.0, controller.cell_gl_size, controller.cell_gl_size,                      0.0f, 1.0f, 0.0f,  // 3
        controller.cell_gl_size, controller.cell_gl_size, 0.0,                      0.0f, 1.0f, 0.0f,  // 6
        controller.cell_gl_size, controller.cell_gl_size, controller.cell_gl_size,  0.0f, 1.0f, 0.0f,  // 7

        0.0, controller.cell_gl_size, 0.0,                                          0.0, 0.0, -1.0, // 2
        0.0, 0.0, 0.0,                                                              0.0, 0.0, -1.0, // 0
        controller.cell_gl_size, 0.0, 0.0,                                          0.0, 0.0, -1.0, // 4
        0.0, controller.cell_gl_size, 0.0,                                          0.0, 0.0, -1.0, // 2
        controller.cell_gl_size, 0.0, 0.0,                                          0.0, 0.0, -1.0, // 4
        controller.cell_gl_size, controller.cell_gl_size, 0.0,                      0.0, 0.0, -1.0, // 6

        controller.cell_gl_size, controller.cell_gl_size, 0.0,                      1.0, 0.0, 0.0,  // 6
        controller.cell_gl_size, 0.0, 0.0,                                          1.0, 0.0, 0.0,  // 4
        controller.cell_gl_size, 0.0, controller.cell_gl_size,                      1.0, 0.0, 0.0,  // 5
        controller.cell_gl_size, controller.cell_gl_size, 0.0,                      1.0, 0.0, 0.0,  // 6
        controller.cell_gl_size, 0.0, controller.cell_gl_size,                      1.0, 0.0, 0.0,  // 5
        controller.cell_gl_size, controller.cell_gl_size, controller.cell_gl_size,  1.0, 0.0, 0.0,  // 7

        controller.cell_gl_size, controller.cell_gl_size, controller.cell_gl_size,  0.0, 0.0, 1.0,  // 7
        controller.cell_gl_size, 0.0, controller.cell_gl_size,                      0.0, 0.0, 1.0,  // 5
        0.0, 0.0, controller.cell_gl_size,                                          0.0, 0.0, 1.0,  // 1
        controller.cell_gl_size, controller.cell_gl_size, controller.cell_gl_size,  0.0, 0.0, 1.0,  // 7
        0.0, 0.0, controller.cell_gl_size,                                          0.0, 0.0, 1.0,  // 1
        0.0, controller.cell_gl_size, controller.cell_gl_size,                      0.0, 0.0, 1.0,  // 3

        0.0, controller.cell_gl_size, controller.cell_gl_size,                      -1.0, 0.0, 0.0, // 3
        0.0, 0.0, controller.cell_gl_size,                                          -1.0, 0.0, 0.0, // 1
        0.0, 0.0, 0.0,                                                              -1.0, 0.0, 0.0, // 0
        0.0, 0.0, controller.cell_gl_size,                                          -1.0, 0.0, 0.0, // 1
        0.0, 0.0, 0.0,                                                              -1.0, 0.0, 0.0, // 0
        0.0, controller.cell_gl_size, 0.0,                                          -1.0, 0.0, 0.0  // 2
    };

    float big_cube[] = {
        -0.8f, -0.8f, -0.8f,    1.0f, 1.0f, 1.0f,   // 0
        -0.8f, -0.8f, 0.8f,     1.0f, 1.0f, 1.0f,   // 1
        -0.8f, 0.8f, -0.8f,     1.0f, 1.0f, 1.0f,   // 2
        -0.8f, 0.8f, 0.8f,      1.0f, 1.0f, 1.0f,   // 3
        0.8f, 0.8f, 0.8f,       1.0f, 1.0f, 1.0f,   // 4
        0.8f, 0.8f, -0.8f,      1.0f, 1.0f, 1.0f,   // 5
        0.8f, -0.8f, 0.8f,      1.0f, 1.0f, 1.0f,   // 6
        0.8f, -0.8f, -0.8f,     1.0f, 1.0f, 1.0f    // 7
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
    controller.bind_load_static_buffer(VBOs, VAOs, sizeof(float)*grid.size(), grid_lines_vertices, 0);

    /*binding second array: just a quad*/
    controller.bind_load_normals_buffer(VBOs, VAOs, sizeof(new_cube_vertices), new_cube_vertices, 1);

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

    controller.bind_load_indices_buffer(VBOs, VAOs, EBO, sizeof(big_cube), big_cube, sizeof(big_cube_indice), big_cube_indice, 2);

    //randomize_lighted_cells(points);

    glm::mat4 projection_3d    = glm::mat4(1.0f);
    glm::mat4 view             = glm::mat4(1.0f);
    glm::mat4 projection       = glm::mat4(1.0f);

    projection_3d = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        
    
    /*Rendering function*/
    auto render = [&](){

        /*background color*/
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*update some state variables*/
        controller.camera.update();
        controller.update_light_cells(points_3d);

        /* get uniform locations */
        unsigned int viewLoc  = glGetUniformLocation(grid_shader_program, "view");
        unsigned int projLoc  = glGetUniformLocation(grid_shader_program, "projection");
        unsigned int color_loc = glGetUniformLocation(shader_program_3d, "uniColor");
        unsigned int intensity_loc = glGetUniformLocation(shader_program_3d, "light_intensity");
        unsigned int coloring_loc = glGetUniformLocation(shader_program_3d, "coloring");
        unsigned int style_3d_loc = glGetUniformLocation(shader_program_3d, "style_3d");
        unsigned int light_cells_Loc = glGetUniformLocation(shader_program_3d, "light_cells");
        unsigned int light_cells_intensity_Loc = glGetUniformLocation(shader_program_3d, "light_cells_intensity");
        unsigned int number_light_cells_Loc = glGetUniformLocation(shader_program_3d, "number_light_cells");


        /*draw background lines*/
        glUseProgram(grid_shader_program);
        if(controller.style_3d == 1){ /*draws cube when in 3d*/
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(controller.camera.get_camera_view()));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_3d));
            glBindVertexArray(VAOs[2]);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        }
        else{                           /*and a grid when in 2d*/
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(VAOs[0]);
            glDrawArrays(GL_LINES, 0, grid.size()/3);
        }

        glBindVertexArray(0);
        
        /*if not in pause*/
        if(controller.running){
            /*Calculating conway step*/
            calculateStep(controller.rows, controller.cols, controller.planes, controller.q_3d, controller.next_state, controller.style_3d == 1);
        }
        
        /*updates the positions buffer and gets the number of active cells*/
        int number_of_active_cells = controller.update_with_step(instanceVBO, controller.next_state, points_3d, controller.rows, controller.cols, controller.planes);

        /*updates this with another shader program*/
        viewLoc  = glGetUniformLocation(shader_program_3d, "view");
        projLoc  = glGetUniformLocation(shader_program_3d, "projection");

        /*draws cells*/
        glUseProgram(shader_program_3d);
        if(controller.style_3d == 1){
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(controller.camera.get_camera_view()));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_3d));
        }
        else{
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        glUniform3fv(color_loc, 1, controller.cell_color);
        glUniform1f(intensity_loc, controller.sun_intensity);
        glUniform1i(coloring_loc, controller.coloring_style);
        glUniform1i(style_3d_loc, controller.style_3d);
        glUniform1fv(light_cells_Loc, controller.number_of_light_cells*3, controller.lighted_cells_positions.data());
        glUniform1f(light_cells_intensity_Loc, controller.light_cells_intensity);
        glUniform1i(number_light_cells_Loc, controller.number_of_light_cells);


        glBindVertexArray(VAOs[1]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 30, number_of_active_cells); /*actually draws*/
        
        /*draws Imgui interface*/
        controller.renderImgui(window.m_glfwWindow, (*window.io));
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glBindVertexArray(0);
        glfwSwapBuffers(window.m_glfwWindow);
    };


    /*Window loop, controls the speed for the simulation*/
    float last_time = glfwGetTime();
    float last_update_time = glfwGetTime();
    float accumulated_time = 0.f;


    while (!glfwWindowShouldClose(window.m_glfwWindow))
    {
        float current_time = glfwGetTime();
        float dt = current_time - last_time;
        last_time = current_time;
        accumulated_time += dt;

        glfwPollEvents();   
        

        /*rendering*/
        float FRAME_TIME = 1.f / (float)controller.current_fps;
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
    glDeleteProgram(grid_shader_program);

    glfwTerminate();
    return 0;   
  
}
