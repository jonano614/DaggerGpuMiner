#include <conio.h>
#include <thread>
#include <fstream>
#include <iostream>
#include "Core\MinerManager.h"

using namespace XDag;

int main(int argc, char** argv)
{
    // Set env vars controlling GPU driver behavior.
    SetEnv("GPU_MAX_HEAP_SIZE", "100");
    SetEnv("GPU_MAX_ALLOC_PERCENT", "100");
    SetEnv("GPU_SINGLE_ALLOC_PERCENT", "100");

    MinerManager miner(MinerManager::OperationMode::None);

    for(int i = 1; i < argc; ++i)
    {
        // Mining options:
        if(!miner.InterpretOption(i, argc, argv))
        {
            std::cerr << "Invalid argument: " << argv[i] << std::endl;
            exit(-1);
        }
    }

    if (!miner.CheckMandatoryParams())
    {
        std::cerr << "Invalid arguments" << std::endl;
        exit(-1);
    }

    miner.Execute();

#if _DEBUG
    //pause and wait
    _getch();
#endif
    return 0;
}