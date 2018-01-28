# DaggerGpuMiner
Stand-alone GPU/CPU miner for Dagger coin

This miner does not require files wallet.dat and dnet_key.dat. Storage folder is still necessary.

Launch parameters:
1) GPU benchmark: DaggerGpuMiner.exe -G -M
2) GPU mining: DaggerGpuMiner.exe -G -a <WALLET_ADDRESS> -p <POOL_ADDRESS>
3) CPU mining: DaggerGpuMiner.exe -cpu -a <WALLET_ADDRESS> -p <POOL_ADDRESS> -t 8

Different features and optional parametes:
1) "-h" - show help
2) you can list all available devices using parameters "-list-devices -G"
3) by default GPU mining is performed only on the first OpenCL device. You can specify several devices using parameter "-opencl-devices 0 1 3". Use your device numbers instead of "0 1 3". Also use can use parameter "-d <N>" there <N> is count of used devices.

The project supports only Windows OS now. Linux and Mac OS are planned in the future.

The project has 3 dependencies: OpenCL, Boost and OpenSSL. 
Windows:
OpenCL SDK can be downloaded by link https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/. Also you can try another OpenCL SDK vendors. Path to intalled SDK should be written to a new environment variable OPENCL_SDK.
Boost and OpenSSL libraries are included by Nuget Manager and should be downloaded automatically.
