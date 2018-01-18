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
            cerr << "Invalid argument: " << argv[i] << endl;
            exit(-1);
        }
    }

    miner.Execute();

    _getch();
    return 0;
}