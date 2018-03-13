# DaggerGpuMiner
Stand-alone GPU/CPU miner for Dagger coin

This miner does not require files wallet.dat and dnet_key.dat. Storage folder is still necessary.

<b>Launch parameters:</b>
1) GPU benchmark: DaggerGpuMiner.exe -G -M
2) GPU mining: DaggerGpuMiner.exe -G -a <WALLET_ADDRESS> -p <POOL_ADDRESS>
3) CPU mining: DaggerGpuMiner.exe -cpu -a <WALLET_ADDRESS> -p <POOL_ADDRESS> -t 8

<b>Different features and optional parameters:</b>
1) "-h" - show help
2) you can list all available devices using parameters "-list-devices -G". You can check what platform ids and device numbers they have.
3) by default GPU-miner uses all OpenCL devices on the selected platform. You can specify particular devices using parameter "-opencl-devices 0 1 3". Use your device numbers instead of "0 1 3". Also use can use parameter "-d <N>" there <N> is count of used devices.
4) if GPU-miner sees only one device, but you have several devices, try to specify platform. For example: "-opencl-platform 1".

The project supports Windows OS, Linux OS and Mac OS now. Mac OS binary file will be provided in the near future.

<b>How to compile the project yourself:</b>  
The project has 3 dependencies: OpenCL, Boost and OpenSSL  
Windows:  
OpenCL SDK can be downloaded by link https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/. Also you can try another OpenCL SDK vendors. Path to intalled SDK should be written to a new environment variable OPENCL_SDK.  
Boost and OpenSSL libraries are included by Nuget Manager and should be downloaded automatically.

Linux:  
AMD driver / SDK link https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/  
Nvidia driver / SDK link https://developer.nvidia.com/cuda-downloads

Check dependencies: libboost-dev, libboost-system-dev, openssl.  

Download this source code, then cd to source folder. In GpuMiner folder run command $make all, it will generate xdag-gpu.  
Launch parameters:
1) GPU benchmark: ./xdag-gpu -G -M
2) GPU mining: ./xdag-gpu -G -a <WALLET_ADDRESS> -p <POOL_ADDRESS>
3) CPU mining: ./xdag-gpu -cpu -a <WALLET_ADDRESS> -p <POOL_ADDRESS> -t 8

Mac OS:  
Install Boost with brew.
Open XCode project to build Mac OS version.

Launch parameters:
1) GPU benchmark: ./xdag-gpu -G -M
2) GPU mining: ./xdag-gpu -G -a <WALLET_ADDRESS> -p <POOL_ADDRESS>
3) CPU mining: ./xdag-gpu -cpu -a <WALLET_ADDRESS> -p <POOL_ADDRESS> -t 8

<b>Workaround on issue with high CPU usage with NVIDIA GPUs.</b>  
There is an issue with NVIDIA GPUs leading to very high CPU usage. The reason is improper implementation of OpenCL by NVIDIA. When CPU thread waits for results from GPU, it does not stop, it spins in loop eating CPU resources for nothing.  
There was impemented a workaround on this issue: before reading results from GPU current thread sleeps during small calculated time. CPU usage was decreased in 90%. The change made optional, use launch parameter "-nvidia-fix" to enable it. The change can decrease hashrate a bit in some cases. But GPU rigs should gain increase of hashrate. So try it and choose to use or not to use it.

You can support author: XDAG  gKNRtSL1pUaTpzMuPMznKw49ILtP6qX3
