// Some global initializations
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "XGlobal.h"
#include "XAddress.h"
#include "dar/crc.h"

void XGlobal::Init()
{
    XAddress::Init();
    crc_init();
}

