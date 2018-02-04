/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#include "CommonData.h"
#include <cstdlib>
#include "Exceptions.h"

using namespace XDag;

int XDag::FromHex(char i, WhenError _throw)
{
    if(i >= '0' && i <= '9')
    {
        return i - '0';
    }
    if(i >= 'a' && i <= 'f')
    {
        return i - 'a' + 10;
    }
    if(i >= 'A' && i <= 'F')
    {
        return i - 'A' + 10;
    }
    if(_throw == WhenError::Throw)
    {
        BOOST_THROW_EXCEPTION(BadHexCharacter() << errinfo_invalidSymbol(i));
    }
    else
    {
        return -1;
    }
}

bool XDag::SetEnv(const char name[], const char value[], bool override)
{
#if _WIN32
    if(!override && std::getenv(name) != nullptr)
        return true;

    return ::_putenv_s(name, value) == 0;
#else
    return ::setenv(name, value, override ? 1 : 0) == 0;
#endif
}
