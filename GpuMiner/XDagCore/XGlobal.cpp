#include "XGlobal.h"
#include "XAddress.h"
#include "dar/crc.h"

void XGlobal::Init()
{
    XAddress::Init();
    crc_init();
}

