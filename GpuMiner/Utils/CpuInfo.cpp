// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "CpuInfo.h"
#include <thread>

unsigned int CpuInfo::GetNumberOfCpuCores()
{
    unsigned int concurentThreadsSupported = std::thread::hardware_concurrency();
    return concurentThreadsSupported;
}
