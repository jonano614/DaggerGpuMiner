/*
   This file is part of ethminer project.
*/
// Licensed under GNU General Public License, Version 3. See the LICENSE file.

// Modified by Evgeniy Sukhomlinov 2018

#include "Miner.h"

XDag::Miner::Miner(std::string const& name, uint32_t index, XTaskProcessor* taskProcessor) :
    Worker(name + std::to_string(index)),
    _index(index),
    _taskProcessor(taskProcessor)
{
}
