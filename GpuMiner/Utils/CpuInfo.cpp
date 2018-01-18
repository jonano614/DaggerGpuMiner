#include "CpuInfo.h"
#include <thread>

unsigned int CpuInfo::GetNumberOfCpuCores()
{
	unsigned int concurentThreadsSupported = std::thread::hardware_concurrency();
	return concurentThreadsSupported;
}
