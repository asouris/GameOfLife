#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdarg>
#include <map>

#include "opencl_conway.h"


using std::chrono::microseconds;

#define block_size 64
#define block_size_2d 8


Queue::Queue(){
    std::cout << "Platform and device info\n";
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    _platform = platforms.front();
    std::cout << "Platform: " << _platform.getInfo<CL_PLATFORM_NAME>()
                << std::endl;

    std::vector<cl::Device> devices;

    _platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    _device = devices.front();
    std::cout << "Device: " << _device.getInfo<CL_DEVICE_NAME>()
                << std::endl;

    std::vector<::size_t> maxWorkItems = _device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    std::cout << "Max sizes: " << maxWorkItems[0] << " " << maxWorkItems[1] << " " << maxWorkItems[2] << std::endl;
    std::cout << "Max group size: " << _device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;

    _context = cl::Context(devices);

    _queue = cl::CommandQueue(_context, _device); 
}

template <typename T>
int Queue::addBuffer(std::vector<T> &data, cl_mem_flags flags){
    cl::Buffer buffer(_context, flags, data.size() * sizeof(T));
    _queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, data.size() * sizeof(T),
                                data.data());
    _buffers.push_back(buffer);
    return _buffers.size() - 1;
}

template <typename T>
void Queue::updateBuffer(std::vector<T> &data, int index){
    _queue.enqueueWriteBuffer(_buffers[index], CL_TRUE, 0, data.size() * sizeof(T),
                                data.data());
}

// Lee el kernel de un archivo
void Queue::setKernel(const std::string &file, const std::string &kernelName)
{
    std::ifstream sourceFile(file);
    std::stringstream sourceCode;
    sourceCode << sourceFile.rdbuf();
    _program = cl::Program(_context, sourceCode.str(), true);

    _kernel = cl::Kernel(_program, kernelName.c_str());
}

template <typename T>
void Queue::readBuffer(std::vector<T> &data, int index)
{
    _queue.enqueueReadBuffer(_buffers[index], CL_TRUE, 0,
                                data.size() * sizeof(T), data.data());
}


template <typename... Args>
cl::Event Queue::operator()(cl::NDRange globalSize, cl::NDRange localSize, Args... args)
{
    for (size_t i = 0; i < _buffers.size(); ++i)
    {
        _kernel.setArg(i, _buffers[i]);
    }

    setKernelArgs(_buffers.size(), args...);

    cl::Event event;
    _queue.enqueueNDRangeKernel(_kernel, cl::NullRange, globalSize, localSize,
                                nullptr, &event);
    event.wait();
    return event;
}


/**
 * Initializes the world with several gliders in different places
 * 
 * @param world array to be initialized
 * @param N size of the array's 2d representation on second dimension
 * @param M size of the array's 2d representation on first dimension
 */
void initWorld(std::vector<int> &world, const int N, const int M){
    const std::vector<std::pair<int,int>> glider = {
		{4, 4},
        {5, 5},
        {6, 3}, {6, 4}, {6, 5}
	};
    int offsets[2] = {10, 15};
    for(auto k : offsets){
        for (auto [i, j] : glider){
		    world[(j + k)%M + i * N] = 1;
            world[j + (i + k)%N * N] = 1;
            world[(j + k)%M + (i + k)%N * N] = 1;
        }
    }
    
}

/**
 * Prints a world to stdout using squares
 */
void printWorld(std::vector < int > &world, int N, int M, int D){
    int j = 0;
    for(int i = 0; i < N*M; i++){
        if(world[i]) std::cout << "■";
        else std::cout << "□";
        if((i+1) % M == 0){
            std::cout << " " << j << std::endl;
            j++;
        }
    }
    std::cout << std::endl;
}

/**
 * Prints a world to stdout using plain values
 */
void printWorldPlain(std::vector < int > &world, int N, int M){
    for(int i = 0; i < N*M; i++){
        std::cout << world[i];
        if((i+1) % M == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
}

/**
 * Prints to stdout
 */
void report(std::string rep){
    std::cout << rep << std::endl;
}

Queue initConway(int N, int M, int type, std::vector<int> &nextState, int D){
    Queue q;
    initWorld(nextState, N, M);

    q.addBuffer(nextState, CL_MEM_READ_ONLY);
    q.addBuffer(nextState, CL_MEM_WRITE_ONLY);

    std::string source;
    if(D == 1) source = type == 0 ? "kernel/CalcStep.cl" : (type == 1 ? "kernel/CalcStep2D.cl" : "kernel/CalcStepGroups.cl");
    else source = "kernel/CalcStep3D.cl";
    q.setKernel(source, "calcStep");

    if(type == 0){
        q.globalSize = cl::NDRange(N * M * D);
        q.localSize = cl::NDRange(block_size);
    }
    else{
        q.globalSize = cl::NDRange(N, M);
        q.localSize = cl::NDRange(block_size_2d, block_size_2d);
    }

    return q;
}


void calculateStep(int N, int M, Queue q, std::vector<int> &nextState, int D, int flag_3d){
    q.updateBuffer(nextState, 0);
    cl::Event event = q(q.globalSize, q.localSize, N, M, D, flag_3d);
    event.wait();
    q.readBuffer(nextState, 1);
}
