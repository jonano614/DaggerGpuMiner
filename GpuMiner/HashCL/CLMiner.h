/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include "Core\Worker.h"
#include "Core\Miner.h"
//#include <libhwmon/wrapnvml.h>
//#include <libhwmon/wrapadl.h>
#if defined(__linux)
#include <libhwmon/wrapamdsysfs.h>
#endif

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS true
#define CL_HPP_ENABLE_EXCEPTIONS true
#define CL_HPP_CL_1_2_DEFAULT_BUILD true
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include "CL/cl2.hpp"

// macOS OpenCL fix:
#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV       0x4000
#endif

#ifndef CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV
#define CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV       0x4001
#endif

#define OPENCL_PLATFORM_UNKNOWN 0
#define OPENCL_PLATFORM_NVIDIA  1
#define OPENCL_PLATFORM_AMD     2
#define OPENCL_PLATFORM_CLOVER  3

namespace XDag
{
    class CLMiner : public Miner
    {
    public:
        /* -- default values -- */
        /// Default value of the local work size. Also known as workgroup size.
        static const unsigned _defaultLocalWorkSize = 128;
        /// Default value of the global work size as a multiplier of the local work size
        static const unsigned _defaultGlobalWorkSizeMultiplier = 8192;

        CLMiner(unsigned index, XTaskProcessor* taskProcessor);
        virtual ~CLMiner();

        static unsigned Instances() { return _numInstances > 0 ? _numInstances : 1; }
        static unsigned GetNumDevices();
        static void ListDevices(bool useAllOpenCLCompatibleDevices);
        static bool ConfigureGPU(
            unsigned localWorkSize,
            unsigned globalWorkSizeMultiplier,
            unsigned platformId,
            bool useAllOpenCLCompatibleDevices
        );
        static void SetNumInstances(unsigned instances) { _numInstances = std::min<unsigned>(instances, GetNumDevices()); }
        static void SetDevices(unsigned * devices, unsigned selectedDeviceCount)
        {
            for(unsigned i = 0; i < selectedDeviceCount; i++)
            {
                _devices[i] = devices[i];
            }
        }
        HwMonitor Hwmon() override;
    protected:
        void KickOff() override;
        void Pause() override;

    private:
        void WorkLoop() override;

        bool Init();
        bool LoadKernel();

        void SetMinShare(XTaskWrapper* taskWrapper, uint64_t* searchBuffer, cheatcoin_field& last);

        cl::Context _context;
        cl::CommandQueue _queue;
        cl::Kernel _searchKernel;
        cl::Buffer _stateBuffer;
        cl::Buffer _dataBuffer;
        cl::Buffer _minHashBuffer;
        cl::Buffer _searchBuffer;
        unsigned _globalWorkSize = 0;
        unsigned _workgroupSize = 0;
        std::string _kernelCode;

        static unsigned _platformId;
        static unsigned _numInstances;
        static std::string _clKernelName;
        static int _devices[16];
        static bool _useAllOpenCLCompatibleDevices;

        /// The local work size for the search
        static unsigned _sWorkgroupSize;
        /// The initial global work size for the searches
        static unsigned _sInitialGlobalWorkSize;

        //wrap_nvml_handle *nvmlh = NULL;
        //wrap_adl_handle *adlh = NULL;
#if defined(__linux)
        wrap_amdsysfs_handle *sysfsh = NULL;
#endif
    };
}
