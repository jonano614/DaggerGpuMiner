# DaggerGpuMiner
Stand-alone GPU/CPU miner for Dagger coin

This miner does not require files wallet.dat and dnet_key.dat. Storage folder is still necessary.

Launch parameters:
1) GPU benchmark: DaggerGpuMiner.exe -G -M
1) GPU mining: DaggerGpuMiner.exe -G -a <WALLET_ADDRESS> -p <POOL_ADDRESS>
1) CPU mining: DaggerGpuMiner.exe -cpu -a <WALLET_ADDRESS> -p <POOL_ADDRESS> -t 8

The project supports only Windows OS now. Linux and Mac OS are planned in the future.

The project has 3 dependencies: OpenCL, Boost and OpenSSL. 
Windows:
OpenCL SDK can be downloaded by link https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/. Also you can try another OpenCL SDK vendors. Path to intalled SDK should be written to a new environment variable OPENCL_SDK.
Boost and OpenSSL libraries are included by Nuget Manager and should be downloaded automatically.
