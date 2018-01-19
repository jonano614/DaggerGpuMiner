#include "XBlock.h"
#if _WIN32
#include "win\sys\time.h"
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAIN_TIME(t)		((t) >> 16)

#if _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

#define CHEATCOIN_MAIN_ERA	0x16940000000ll

#define STORAGE_DIR0			"storage%s"
#define STORAGE_DIR0_ARGS(t)	""
#define STORAGE_DIR1			STORAGE_DIR0 SLASH "%02x"
#define STORAGE_DIR1_ARGS(t)	STORAGE_DIR0_ARGS(t), (int)((t) >> 40)
#define STORAGE_DIR2			STORAGE_DIR1 SLASH "%02x"
#define STORAGE_DIR2_ARGS(t)	STORAGE_DIR1_ARGS(t), (int)((t) >> 32) & 0xff
#define STORAGE_DIR3			STORAGE_DIR2 SLASH "%02x"
#define STORAGE_DIR3_ARGS(t)	STORAGE_DIR2_ARGS(t), (int)((t) >> 24) & 0xff
#define STORAGE_FILE			STORAGE_DIR3 SLASH "%02x.dat"
#define STORAGE_FILE_ARGS(t)	STORAGE_DIR3_ARGS(t), (int)((t) >> 16) & 0xff
#define SUMS_FILE				"sums.dat"

static uint64_t get_timestamp(void)
{
    timeval tp;
    gettimeofday(&tp, 0);
    return (uint64_t)(unsigned long)tp.tv_sec << 10 | ((tp.tv_usec << 10) / 1000000);
}

cheatcoin_time_t XBlock::GetMainTime()
{
    return MAIN_TIME(get_timestamp());
}

//storage folder contains a file with data necessary for communication with pool
//it is the first created *.dat file in that folder
bool XBlock::GetFirstBlock(cheatcoin_block *firstBlock)
{
    cheatcoin_time_t start_time = CHEATCOIN_MAIN_ERA;
    cheatcoin_time_t end_time = get_timestamp();

    struct stat st;
    char path[256];
    FILE *f;
    uint64_t mask;
    bool found = false;
    while (start_time < end_time)
    {
        sprintf(path, STORAGE_FILE, STORAGE_FILE_ARGS(start_time));
        f = fopen(path, "rb");
        if (f)
        {
            if (fseek(f, 0, SEEK_SET) >= 0)
            {
                fread(firstBlock, sizeof(struct cheatcoin_block), 1, f);
                found = true;
            }
            fclose(f);

            if (found)
            {
                break;
            }
        }
        if (sprintf(path, STORAGE_DIR3, STORAGE_DIR3_ARGS(start_time)), !stat(path, &st))
        {
            mask = (1l << 16) - 1;
        }
        else if (sprintf(path, STORAGE_DIR2, STORAGE_DIR2_ARGS(start_time)), !stat(path, &st))
        {
            mask = (1l << 24) - 1;
        }
        else if (sprintf(path, STORAGE_DIR1, STORAGE_DIR1_ARGS(start_time)), !stat(path, &st))
        {
            mask = (1ll << 32) - 1;
        }
        else
        {
            mask = (1ll << 40) - 1;
        }
        start_time |= mask;
        start_time++;
    }
    return found;
}