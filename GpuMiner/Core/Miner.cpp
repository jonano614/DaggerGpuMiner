#include "Miner.h"

XDag::Miner::Miner(std::string const& _name, size_t index, XTaskProcessor* taskProcessor) :
    Worker(_name + std::to_string(index)),
    _index(index),
    _taskProcessor(taskProcessor)
{
}