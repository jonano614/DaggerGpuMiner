#ifdef _WIN32
#include <conio.h>
#endif
#include <thread>
#include <fstream>
#include <iostream>
#include "Core/Log.h"
#include "Core/MinerManager.h"

using namespace XDag;

void Help()
{
    std::cout
        << "Usage Dagger Gpu Miner [OPTIONS]" << std::endl
        << "Options:" << std::endl << std::endl;
    MinerManager::StreamHelp(std::cout);
    std::cout
        << "General Options:" << std::endl
        << "    -v,--verbosity <0 - 9>  Set the log verbosity from 0 to 9 (default: 8)." << std::endl
        << "    -h,--help  Show this help message and exit." << std::endl
        ;
    exit(0);
}

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
        if(miner.InterpretOption(i, argc, argv))
        {
            continue;
        }

        std::string arg = argv[i];
        if((arg == "-v" || arg == "--verbosity") && i + 1 < argc)
        {
            g_logVerbosity = atoi(argv[++i]);
        }
        else if(arg == "-h" || arg == "--help")
        {
            Help();
        }
        else
        {
            std::cerr << "Invalid argument: " << arg << std::endl;
            exit(-1);
        }
    }

    if(!miner.CheckMandatoryParams())
    {
        std::cerr << "Invalid arguments" << std::endl;
        exit(-1);
    }

    miner.Execute();

#ifdef _DEBUG
    //pause and wait
    _getch();
#endif
    return 0;
}
