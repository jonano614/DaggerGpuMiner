/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#include "CLMiner.h"
#include "Utils\PathUtils.h"
#include <fstream>

using namespace XDag;

unsigned CLMiner::_sWorkgroupSize = CLMiner::_defaultLocalWorkSize;
unsigned CLMiner::_sInitialGlobalWorkSize = CLMiner::_defaultGlobalWorkSizeMultiplier * CLMiner::_defaultLocalWorkSize;
std::string CLMiner::_clKernelName = "CLMiner_kernel.cl";

constexpr size_t c_maxSearchResults = 1;

struct CLChannel : public LogChannel
{
    static const char* name() { return EthOrange " cl"; }
    static const int verbosity = 2;
    static const bool debug = false;
};
#define cllog clog(CLChannel)
#define ETHCL_LOG(_contents) cllog << _contents

/**
 * Returns the name of a numerical cl_int error
 * Takes constants from CL/cl.h and returns them in a readable format
 */
static const char *strClError(cl_int err)
{

    switch(err)
    {
    case CL_SUCCESS:
        return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND:
        return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE:
        return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE:
        return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES:
        return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY:
        return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP:
        return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE:
        return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE:
        return "CL_MAP_FAILURE";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";

#ifdef CL_VERSION_1_2
    case CL_COMPILE_PROGRAM_FAILURE:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case CL_LINKER_NOT_AVAILABLE:
        return "CL_LINKER_NOT_AVAILABLE";
    case CL_LINK_PROGRAM_FAILURE:
        return "CL_LINK_PROGRAM_FAILURE";
    case CL_DEVICE_PARTITION_FAILED:
        return "CL_DEVICE_PARTITION_FAILED";
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
#endif // CL_VERSION_1_2

    case CL_INVALID_VALUE:
        return "CL_INVALID_VALUE";
    case CL_INVALID_DEVICE_TYPE:
        return "CL_INVALID_DEVICE_TYPE";
    case CL_INVALID_PLATFORM:
        return "CL_INVALID_PLATFORM";
    case CL_INVALID_DEVICE:
        return "CL_INVALID_DEVICE";
    case CL_INVALID_CONTEXT:
        return "CL_INVALID_CONTEXT";
    case CL_INVALID_QUEUE_PROPERTIES:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE:
        return "CL_INVALID_COMMAND_QUEUE";
    case CL_INVALID_HOST_PTR:
        return "CL_INVALID_HOST_PTR";
    case CL_INVALID_MEM_OBJECT:
        return "CL_INVALID_MEM_OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE:
        return "CL_INVALID_IMAGE_SIZE";
    case CL_INVALID_SAMPLER:
        return "CL_INVALID_SAMPLER";
    case CL_INVALID_BINARY:
        return "CL_INVALID_BINARY";
    case CL_INVALID_BUILD_OPTIONS:
        return "CL_INVALID_BUILD_OPTIONS";
    case CL_INVALID_PROGRAM:
        return "CL_INVALID_PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case CL_INVALID_KERNEL_NAME:
        return "CL_INVALID_KERNEL_NAME";
    case CL_INVALID_KERNEL_DEFINITION:
        return "CL_INVALID_KERNEL_DEFINITION";
    case CL_INVALID_KERNEL:
        return "CL_INVALID_KERNEL";
    case CL_INVALID_ARG_INDEX:
        return "CL_INVALID_ARG_INDEX";
    case CL_INVALID_ARG_VALUE:
        return "CL_INVALID_ARG_VALUE";
    case CL_INVALID_ARG_SIZE:
        return "CL_INVALID_ARG_SIZE";
    case CL_INVALID_KERNEL_ARGS:
        return "CL_INVALID_KERNEL_ARGS";
    case CL_INVALID_WORK_DIMENSION:
        return "CL_INVALID_WORK_DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case CL_INVALID_WORK_ITEM_SIZE:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case CL_INVALID_GLOBAL_OFFSET:
        return "CL_INVALID_GLOBAL_OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case CL_INVALID_EVENT:
        return "CL_INVALID_EVENT";
    case CL_INVALID_OPERATION:
        return "CL_INVALID_OPERATION";
    case CL_INVALID_GL_OBJECT:
        return "CL_INVALID_GL_OBJECT";
    case CL_INVALID_BUFFER_SIZE:
        return "CL_INVALID_BUFFER_SIZE";
    case CL_INVALID_MIP_LEVEL:
        return "CL_INVALID_MIP_LEVEL";
    case CL_INVALID_GLOBAL_WORK_SIZE:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case CL_INVALID_PROPERTY:
        return "CL_INVALID_PROPERTY";

#ifdef CL_VERSION_1_2
    case CL_INVALID_IMAGE_DESCRIPTOR:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case CL_INVALID_COMPILER_OPTIONS:
        return "CL_INVALID_COMPILER_OPTIONS";
    case CL_INVALID_LINKER_OPTIONS:
        return "CL_INVALID_LINKER_OPTIONS";
    case CL_INVALID_DEVICE_PARTITION_COUNT:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif // CL_VERSION_1_2

#ifdef CL_VERSION_2_0
    case CL_INVALID_PIPE_SIZE:
        return "CL_INVALID_PIPE_SIZE";
    case CL_INVALID_DEVICE_QUEUE:
        return "CL_INVALID_DEVICE_QUEUE";
#endif // CL_VERSION_2_0

#ifdef CL_VERSION_2_2
    case CL_INVALID_SPEC_ID:
        return "CL_INVALID_SPEC_ID";
    case CL_MAX_SIZE_RESTRICTION_EXCEEDED:
        return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif // CL_VERSION_2_2
    }

    return "Unknown CL error encountered";
}

/**
 * Prints cl::Errors in a uniform way
 * @param msg text prepending the error message
 * @param clerr cl:Error object
 *
 * Prints errors in the format:
 *      msg: what(), string err() (numeric err())
 */
static std::string XDagCLErrorHelper(const char *msg, cl::Error const &clerr)
{
    std::ostringstream osstream;
    osstream << msg << ": " << clerr.what() << ": " << strClError(clerr.err())
        << " (" << clerr.err() << ")";
    return osstream.str();
}

void AddDefinition(std::string& _source, char const* _id, unsigned _value)
{
    char buf[256];
    sprintf(buf, "#define %s %uu\n", _id, _value);
    _source.insert(_source.begin(), buf, buf + strlen(buf));
}

std::vector<cl::Platform> GetPlatforms()
{
    std::vector<cl::Platform> platforms;
    try
    {
        cl::Platform::get(&platforms);
    }
    catch(cl::Error const& err)
    {
#if defined(CL_PLATFORM_NOT_FOUND_KHR)
        if(err.err() == CL_PLATFORM_NOT_FOUND_KHR)
            cwarn << "No OpenCL platforms found";
        else
#endif
            throw err;
    }
    return platforms;
}

std::vector<cl::Device> GetDevices(std::vector<cl::Platform> const& _platforms, unsigned _platformId)
{
    std::vector<cl::Device> devices;
    size_t platform_num = std::min<size_t>(_platformId, _platforms.size() - 1);
    try
    {
        _platforms[platform_num].getDevices(
            CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR,
            &devices
        );
    }
    catch(cl::Error const& err)
    {
        // if simply no devices found return empty vector
        if(err.err() != CL_DEVICE_NOT_FOUND)
            throw err;
    }
    return devices;
}


unsigned CLMiner::_platformId = 0;
unsigned CLMiner::_numInstances = 0;
int CLMiner::_devices[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

CLMiner::CLMiner(unsigned index, XTaskProcessor* taskProcessor)
    :Miner("cl-", index, taskProcessor)
{
}

CLMiner::~CLMiner()
{
    Pause();
}

bool CLMiner::ConfigureGPU(
    unsigned _localWorkSize,
    unsigned _globalWorkSizeMultiplier,
    unsigned _platformId
)
{
    //s_dagLoadMode = _dagLoadMode;
    //s_dagCreateDevice = _dagCreateDevice;

    _platformId = _platformId;

    _localWorkSize = ((_localWorkSize + 7) / 8) * 8;
    _sWorkgroupSize = _localWorkSize;
    _sInitialGlobalWorkSize = _globalWorkSizeMultiplier * _localWorkSize;

    //uint64_t dagSize = ethash_get_datasize(_currentBlock);

    std::vector<cl::Platform> platforms = GetPlatforms();
    if(platforms.empty())
    {
        return false;
    }
    if(_platformId >= platforms.size())
    {
        return false;
    }

    std::vector<cl::Device> devices = GetDevices(platforms, _platformId);
    for(auto const& device : devices)
    {
        cl_ulong result = 0;
        device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &result);
        //if (result >= dagSize)
        //{
        cnote <<
            "Found suitable OpenCL device [" << device.getInfo<CL_DEVICE_NAME>()
            << "] with " << result << " bytes of GPU memory";
        return true;
        //}
    }

    //std::cout << "No GPU device with sufficient memory was found. Can't GPU mine. Remove the -G argument" << endl;
    return false;
}

bool CLMiner::Init()
{
    // get all platforms
    try
    {
        if(!LoadKernel())
        {
            return false;
        }

        std::vector<cl::Platform> platforms = GetPlatforms();
        if(platforms.empty())
        {
            return false;
        }

        // use selected platform
        unsigned platformIdx = std::min<unsigned>(_platformId, platforms.size() - 1);

        std::string platformName = platforms[platformIdx].getInfo<CL_PLATFORM_NAME>();
        ETHCL_LOG("Platform: " << platformName);

        int platformId = OPENCL_PLATFORM_UNKNOWN;
        {
            // this mutex prevents race conditions when calling the adl wrapper since it is apparently not thread safe
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);

            if(platformName == "NVIDIA CUDA")
            {
                platformId = OPENCL_PLATFORM_NVIDIA;
                //nvmlh = wrap_nvml_create();
            }
            else if(platformName == "AMD Accelerated Parallel Processing")
            {
                platformId = OPENCL_PLATFORM_AMD;
                //adlh = wrap_adl_create();
#if defined(__linux)
                sysfsh = wrap_amdsysfs_create();
#endif
            }
            else if(platformName == "Clover")
            {
                platformId = OPENCL_PLATFORM_CLOVER;
            }
        }

        // get GPU device of the default platform
        std::vector<cl::Device> devices = GetDevices(platforms, platformIdx);
        if(devices.empty())
        {
            ETHCL_LOG("No OpenCL devices found.");
            return false;
        }

        // use selected device
        unsigned deviceId = _devices[_index] > -1 ? _devices[_index] : _index;
        cl::Device& device = devices[std::min<unsigned>(deviceId, devices.size() - 1)];
        std::string device_version = device.getInfo<CL_DEVICE_VERSION>();
        ETHCL_LOG("Device:   " << device.getInfo<CL_DEVICE_NAME>() << " / " << device_version);

        std::string clVer = device_version.substr(7, 3);
        if(clVer == "1.0" || clVer == "1.1")
        {
            if(platformId == OPENCL_PLATFORM_CLOVER)
            {
                ETHCL_LOG("OpenCL " << clVer << " not supported, but platform Clover might work nevertheless. USE AT OWN RISK!");
            }
            else
            {
                ETHCL_LOG("OpenCL " << clVer << " not supported - minimum required version is 1.2");
                return false;
            }
        }

        char options[256];
        int computeCapability = 0;
        if(platformId == OPENCL_PLATFORM_NVIDIA)
        {
            cl_uint computeCapabilityMajor;
            cl_uint computeCapabilityMinor;
            clGetDeviceInfo(device(), CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(cl_uint), &computeCapabilityMajor, NULL);
            clGetDeviceInfo(device(), CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(cl_uint), &computeCapabilityMinor, NULL);

            computeCapability = computeCapabilityMajor * 10 + computeCapabilityMinor;
            int maxregs = computeCapability >= 35 ? 72 : 63;
            sprintf(options, "-cl-nv-maxrregcount=%d", maxregs);
        }
        else
        {
            sprintf(options, "%s", "");
        }
        // create context
        _context = cl::Context(std::vector<cl::Device>(&device, &device + 1));
        _queue = cl::CommandQueue(_context, device);

        // make sure that global work size is evenly divisible by the local workgroup size
        _workgroupSize = _sWorkgroupSize;
        _globalWorkSize = _sInitialGlobalWorkSize;
        if(_globalWorkSize % _workgroupSize != 0)
        {
            _globalWorkSize = ((_globalWorkSize / _workgroupSize) + 1) * _workgroupSize;
        }

        //uint64_t dagSize = ethash_get_datasize(light->light->block_number);
        //uint32_t dagSize128 = (unsigned)(dagSize / ETHASH_MIX_BYTES);
        //uint32_t lightSize64 = (unsigned)(light->data().size() / sizeof(node));

        // patch source code
        // note: The kernels here are simply compiled version of the respective .cl kernels
        // into a byte array by bin2h.cmake. There is no need to load the file by hand in runtime
        // See libethash-cl/CMakeLists.txt: add_custom_command()
        // TODO: Just use C++ raw string literal.

        AddDefinition(_kernelCode, "GROUP_SIZE", _workgroupSize);
        AddDefinition(_kernelCode, "MAX_OUTPUTS", c_maxSearchResults);
        AddDefinition(_kernelCode, "PLATFORM", platformId);
        AddDefinition(_kernelCode, "COMPUTE", computeCapability);

        // create miner OpenCL program
        cl::Program::Sources sources{ { _kernelCode.data(), _kernelCode.size() } };
        cl::Program program(_context, sources);
        try
        {
            program.build({ device }, options);
            cllog << "Build info:" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
        }
        catch(cl::Error const&)
        {
            cwarn << "Build info:" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
            return false;
        }

        _searchKernel = cl::Kernel(program, "search_nonce");

        // create buffer for initial hashing state
        ETHCL_LOG("Creating buffer for initial hashing state.");
        _stateBuffer = cl::Buffer(_context, CL_MEM_READ_ONLY, 32);

        // create buffer for mininal target hash
        ETHCL_LOG("Creating buffer for target hash.");
        _minHashBuffer = cl::Buffer(_context, CL_MEM_READ_ONLY, 32);

        // create mining buffers
        ETHCL_LOG("Creating output buffer");
        //TODO: buffer size?  What if local work group size?
        _searchBuffer = cl::Buffer(_context, CL_MEM_WRITE_ONLY, _globalWorkSize * sizeof(uint64_t));
    }
    catch(cl::Error const& err)
    {
        cwarn << XDagCLErrorHelper("OpenCL init failed", err);
        return false;
    }
    return true;
}

void CLMiner::WorkLoop()
{
    // Memory for zero-ing buffers. Cannot be static because crashes on macOS.
    uint32_t const c_zero = 0;
    cheatcoin_hash_t hash;
    cheatcoin_field last;
    XTaskWrapper* previousTaskWrapper = 0;
    uint64_t nonce;
    int iterations = 256;

    if(!Init())
    {
        //TODO: error message, think of better place
        return;
    }

    uint64_t* results = new uint64_t[_globalWorkSize];

    try
    {
        while(true)
        {
            XTaskWrapper* taskWrapper = GetTask();
            if(taskWrapper == NULL)
            {
                clog(LogChannel) << "No work. Pause for 2 s.";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            if(previousTaskWrapper == NULL || previousTaskWrapper != taskWrapper)
            {
                previousTaskWrapper = taskWrapper;
                memcpy(last.data, taskWrapper->GetTask()->nonce.data, sizeof(cheatcoin_hash_t));
                nonce = last.amount + _index * 1000000000000;//TODO: think of nonce increment

                // New work received. Update GPU data.
                // Update header constant buffer.
                _queue.enqueueWriteBuffer(_stateBuffer, CL_FALSE, 0, 32, taskWrapper->GetTask()->ctx.state);
                _queue.enqueueWriteBuffer(_minHashBuffer, CL_FALSE, 0, 32, taskWrapper->GetTask()->minhash.data);
                _queue.enqueueWriteBuffer(_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);

                _searchKernel.setArg(0, _stateBuffer);
                _searchKernel.setArg(2, iterations);
                _searchKernel.setArg(3, _minHashBuffer);
                _searchKernel.setArg(4, _searchBuffer); // Supply output buffer to kernel.
            }

            // Read results.
            _queue.enqueueReadBuffer(_searchBuffer, CL_TRUE, 0, _globalWorkSize * sizeof(uint64_t), results);

            //TODO: processing output buffer
            uint64_t minNonce = 0;//GetMinNonce();
            if(minNonce > 0)
            {
                // Reset search buffer if any solution found.
                _queue.enqueueWriteBuffer(_searchBuffer, CL_FALSE, 0, sizeof(c_zero), &c_zero);
            }

            // Increase start nonce for following kernel execution.
            nonce += _globalWorkSize * iterations;

            // Run the kernel.
            _searchKernel.setArg(1, nonce);
            _queue.enqueueNDRangeKernel(_searchKernel, cl::NullRange, _globalWorkSize, _workgroupSize);

            // Report results while the kernel is running.
            // It takes some time because hash must be re-evaluated on CPU.
            if(minNonce > 0)
            {
                //TODO: SetShare
            }

            // Report hash count
            AddHashCount(_globalWorkSize * iterations);

            // Check if we should stop.
            if(ShouldStop())
            {
                // Make sure the last buffer write has finished --
                // it reads local variable.
                _queue.finish();
                break;
            }
        }
    }
    catch(cl::Error const& _e)
    {
        cwarn << XDagCLErrorHelper("OpenCL Error", _e);
    }

    delete []results;
}

void CLMiner::KickOff()
{
}

void CLMiner::Pause()
{
}

unsigned CLMiner::GetNumDevices()
{
    std::vector<cl::Platform> platforms = GetPlatforms();
    if(platforms.empty())
        return 0;

    std::vector<cl::Device> devices = GetDevices(platforms, _platformId);
    if(devices.empty())
    {
        cwarn << "No OpenCL devices found.";
        return 0;
    }
    return devices.size();
}

void CLMiner::ListDevices()
{
    std::string outString = "\nListing OpenCL devices.\nFORMAT: [platformID] [deviceID] deviceName\n";
    unsigned int i = 0;

    std::vector<cl::Platform> platforms = GetPlatforms();
    if(platforms.empty())
        return;
    for(unsigned j = 0; j < platforms.size(); ++j)
    {
        i = 0;
        std::vector<cl::Device> devices = GetDevices(platforms, j);
        for(auto const& device : devices)
        {
            outString += "[" + std::to_string(j) + "] [" + std::to_string(i) + "] " + device.getInfo<CL_DEVICE_NAME>() + "\n";
            outString += "\tCL_DEVICE_TYPE: ";
            switch(device.getInfo<CL_DEVICE_TYPE>())
            {
            case CL_DEVICE_TYPE_CPU:
                outString += "CPU\n";
                break;
            case CL_DEVICE_TYPE_GPU:
                outString += "GPU\n";
                break;
            case CL_DEVICE_TYPE_ACCELERATOR:
                outString += "ACCELERATOR\n";
                break;
            default:
                outString += "DEFAULT\n";
                break;
            }
            outString += "\tCL_DEVICE_GLOBAL_MEM_SIZE: " + std::to_string(device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>()) + "\n";
            outString += "\tCL_DEVICE_MAX_MEM_ALLOC_SIZE: " + std::to_string(device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()) + "\n";
            outString += "\tCL_DEVICE_MAX_WORK_GROUP_SIZE: " + std::to_string(device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()) + "\n";
            ++i;
        }
    }
    std::cout << outString;
}

HwMonitor CLMiner::Hwmon()
{
    HwMonitor hw;
    unsigned int tempC = 0, fanpcnt = 0;
    /*if (nvmlh) {
        wrap_nvml_get_tempC(nvmlh, index, &tempC);
        wrap_nvml_get_fanpcnt(nvmlh, index, &fanpcnt);
    }
    if (adlh) {
        wrap_adl_get_tempC(adlh, index, &tempC);
        wrap_adl_get_fanpcnt(adlh, index, &fanpcnt);
    }*/
#if defined(__linux)
    if(sysfsh)
    {
        wrap_amdsysfs_get_tempC(sysfsh, index, &tempC);
        wrap_amdsysfs_get_fanpcnt(sysfsh, index, &fanpcnt);
    }
#endif
    hw.tempC = tempC;
    hw.fanP = fanpcnt;
    return hw;
}

/* loads the kernel file into a string */
bool CLMiner::LoadKernel()
{
    std::string path = PathUtils::GetModuleFolder();
    path += _clKernelName;
    if(!PathUtils::FileExists(path))
    {
        //TODO: logging
        //std::cout << "Error: kernel file not found: " << filename << std::endl;
        return false;
    }
    size_t size;
    char* str;
    std::fstream f(path, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size + 1];
        if(!str)
        {
            f.close();
            return false;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        _kernelCode = str;
        delete[] str;
        return true;
    }
    //TODO: logging
    //std::cout << "Error: failed to open file: " << filename << std::endl;
    return false;
}
