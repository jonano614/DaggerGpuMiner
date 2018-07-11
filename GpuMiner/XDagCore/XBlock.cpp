// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "XBlock.h"
#include "XTime.h"

void XBlock::GenerateFakeBlock(xdag_block *block)
{
    memset(block, 0, sizeof(struct xdag_block));
    block->field[0].type = (int)xdag_field_type::XDAG_FIELD_HEAD | (uint64_t)((int)xdag_field_type::XDAG_FIELD_SIGN_OUT * 0x11) << 4;
    block->field[0].time = get_timestamp();
    block->field[0].amount = 0;
    block->field[0].transport_header = 1;
}
