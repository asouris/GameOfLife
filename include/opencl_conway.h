#include <vector>

#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>

/** Implements a OpenCL command queue
 *  Manages access and updates on the openCL command queue
 */
class Queue
{
private:
    cl::Platform _platform;
    cl::Device _device;
    cl::Context _context;
    cl::CommandQueue _queue;
    std::vector<cl::Buffer> _buffers;
    cl::Kernel _kernel;
    cl::Program _program;

    void setKernelArgs(int idx) {}

    /** Sets a kernel arg */
    template <typename Last>
    void setKernelArgs(int idx, Last last)
    {
        _kernel.setArg(idx, last);
    };

    /** Sets multiple kernel args */
    template <typename First, typename... Rest>
    void setKernelArgs(int idx, First first, Rest... rest)
    {
        _kernel.setArg(idx, first);
        setKernelArgs(idx + 1, rest...);
    };

public:
    cl::NDRange globalSize; 
    cl::NDRange localSize;

    /** Constructs a Queue */
    Queue();

    /** Adds a new OpenCL buffer
     * @param data vector of data to be written in the buffer
     * @param flags type of buffer
     * @tparam T type of data element
     * @return index of new buffer
    */
    template <typename T>
    int addBuffer(std::vector<T> &data, cl_mem_flags flags = CL_MEM_READ_WRITE);

    /** Updates an existing OpenCL buffer
     * @param data vector of data to be written in the buffer
     * @param index index of the buffer
     * @tparam T type of data element
    */
    template <typename T>
    void updateBuffer(std::vector<T> &data, int index);

    /** Reads and loads kernel
     * @param file path to kernel file
     * @param kernelName name of the kernel function
     */
    void setKernel(const std::string &file, const std::string &kernelName);

    /** Reads an existing OpenCL buffer
     * @param data vector of data to write the buffer
     * @param index index of the buffer to be read
     * @tparam T type of data element
    */
    template <typename T>
    void readBuffer(std::vector<T> &data, int index = 0);

    /** Calls the kernel
     * As arguments uses the already set buffers and the ones in Args
     * @param globalSize total number of threads
     * @param localSize threads per group
     * @param args aditional arguments for the kernel
     * @tparam Args type of the aditional arguments
     * @return A OpenCL event
     */
    template <typename... Args>
    cl::Event operator()(cl::NDRange globalSize, cl::NDRange localSize, Args... args);
};

/** Initializes a Command Queue with everything needed to iterate the Conway's Game
 * @param N amount of rows in the world
 * @param M amount of columns in the world
 * @param type type of kernel implementation to be used
 * @param nextState vector that will hold the next state in the game
 * @return an initialized Queue
 */
Queue initConway(int N, int M, int type, std::vector<int> &nextState);

/** Runs an iteration of the simulation
 * @param N amount of rows in the world
 * @param M amount of columns in the world
 * @param q OpenCL command queue that holds the kernel and buffer references
 * @param nextState vector that will hold the next state in the game
 */
void calculateStep(int N, int M, Queue q, std::vector<int> &nextState);

/** Formats and prints a world state to console
 * @param world vector holding the world state
 * @param N amount of rows in the world
 * @param M amount of columns in the world
 */
void printWorld(std::vector < int > &world, int N, int M);

