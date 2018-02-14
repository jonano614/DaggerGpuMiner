/// OpenCL miner implementation.
///
/// @file
/// @copyright GNU General Public License

#pragma once

#include "Core/Worker.h"
#include "Core/Miner.h"

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

#define MAX_CL_DEVICES 16

namespace XDag
{
    class CLMiner : public Miner
    {
    public:
        /* -- default values -- */
        /// Default value of the local work size. Also known as workgroup size.
        static const uint32_t _defaultLocalWorkSize = 128;
        /// Default value of the global work size as a multiplier of the local work size
        static const uint32_t _defaultGlobalWorkSizeMultiplier = 8192;

        CLMiner(uint32_t index, XTaskProcessor* taskProcessor);
        virtual ~CLMiner();

        static uint32_t Instances() { return _numInstances > 0 ? _numInstances : 1; }
        static uint32_t GetNumDevices();
        static void ListDevices(bool useOpenClCpu);
        static bool ConfigureGPU(
            uint32_t localWorkSize,
            uint32_t globalWorkSizeMultiplier,
            uint32_t platformId,
            bool useOpenClCpu
        );
        static void SetNumInstances(uint32_t instances) { _numInstances = std::min<uint32_t>(instances, GetNumDevices()); }
        static void SetDevices(uint32_t * devices, uint32_t selectedDeviceCount)
        {
            for(uint32_t i = 0; i < selectedDeviceCount; i++)
            {
                _devices[i] = devices[i];
            }
        }
        static void SetUseNvidiaFix(bool useNvidiaFix) { _useNvidiaFix = useNvidiaFix; }

        bool Initialize() override;
        HwMonitor Hwmon() override;

    private:
        void WorkLoop() override;
        bool LoadKernelCode();
        void SetMinShare(XTaskWrapper* taskWrapper, uint64_t* searchBuffer, cheatcoin_field& last);
        void WriteKernelArgs(XTaskWrapper* taskWrapper, uint64_t* zeroBuffer);
        void ReadData(uint64_t* results);

        cl::Context _context;
        cl::CommandQueue _queue;
        cl::Kernel _searchKernel;
        cl::Buffer _stateBuffer;
        cl::Buffer _precalcStateBuffer;
        cl::Buffer _dataBuffer;
        cl::Buffer _searchBuffer;
        uint32_t _globalWorkSize;
        uint32_t _workgroupSize;
        std::string _kernelCode;
        uint32_t _platformId;
        uint32_t _kernelExecutionMcs;

        static uint32_t _selectedPlatformId;
        static uint32_t _numInstances;
        static std::string _clKernelName;
        static int _devices[MAX_CL_DEVICES];
        static bool _useOpenClCpu;
        static bool _useNvidiaFix;

        /// The local work size for the search
        static uint32_t _sWorkgroupSize;
        /// The initial global work size for the searches
        static uint32_t _sInitialGlobalWorkSize;
    };
}
