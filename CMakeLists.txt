
cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(tarea3 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

FetchContent_Declare(glfw URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz)
FetchContent_Declare(glm URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.tar.gz)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

add_library(
        opencl_conway STATIC
        src/opencl/opencl_conway.cpp
)
target_include_directories(opencl_conway PUBLIC ${PROJECT_SOURCE_DIR}/include)

#copy kernels to bin
file(COPY src/opencl/CalcStep.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStep2D.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStepGroups.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStep3D.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)

#copy shaders to bin
file(COPY src/shaders/3d_fragment.glsl DESTINATION ${PROJECT_SOURCE_DIR}/bin/shaders/)
file(COPY src/shaders/3d_vertex.glsl DESTINATION ${PROJECT_SOURCE_DIR}/bin/shaders/)
file(COPY src/shaders/grid_fragment.glsl DESTINATION ${PROJECT_SOURCE_DIR}/bin/shaders/)
file(COPY src/shaders/grid_vertex.glsl DESTINATION ${PROJECT_SOURCE_DIR}/bin/shaders/)

FetchContent_Declare(imgui_external
	URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.0.tar.gz
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(imgui_external)

add_library(imgui
	${imgui_external_SOURCE_DIR}/imgui.cpp
	${imgui_external_SOURCE_DIR}/imgui_draw.cpp
	${imgui_external_SOURCE_DIR}/imgui_tables.cpp
	${imgui_external_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_external_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_external_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp

)
target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})


add_library(
	glad STATIC 
	${PROJECT_SOURCE_DIR}/src/glad/glad.c 
)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/src/glad/include)

add_library(
        utils STATIC
        src/utils.cpp
)
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(utils glad imgui glfw opencl_conway OpenCL glm)

#set(SOURCES src/conway.cpp src/glad.c)
add_executable(conway src/conway.cpp)

target_include_directories(conway PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(conway ${OPENGL_LIBRARIES} utils)

