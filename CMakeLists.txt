cmake_minimum_required(VERSION 3.22)

project(tarea3 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES src/conway.cpp src/glad.c)
add_executable(conway ${SOURCES})

find_package(OpenGL REQUIRED)
add_subdirectory(include/glfw-3.4)
include_directories(${OPENGL_INCLUDE_DIRS})

add_library(
        opencl_conway STATIC
        src/opencl/opencl_conway.cpp
)
target_include_directories(opencl_conway PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_include_directories(conway PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(conway ${OPENGL_LIBRARIES} glfw OpenCL opencl_conway)


# add_executable(opencl src/opencl/opencl_conway.cpp)
# target_include_directories(opencl PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_link_libraries(opencl OpenCL)

#copy kernels to bin
file(COPY src/opencl/CalcStep.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStep2D.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStepGroups.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)
file(COPY src/opencl/CalcStep3D.cl DESTINATION ${PROJECT_SOURCE_DIR}/bin/kernel/)

add_library(
        utils STATIC
        src/utils.cpp
)
target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_executable(seq src/sequential-conway.cpp)
target_link_libraries(seq utils)


