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

std::string XDag::Escaped(std::string const& _s, bool _all)
{
    static const std::map<char, char> prettyEscapes{ {'\r', 'r'}, {'\n', 'n'}, {'\t', 't'}, {'\v', 'v'} };
    std::string ret;
    ret.reserve(_s.size() + 2);
    ret.push_back('"');
    for(auto i : _s)
    {
        if(i == '"' && !_all)
            ret += "\\\"";
        else if(i == '\\' && !_all)
            ret += "\\\\";
        else if(prettyEscapes.count(i) && !_all)
        {
            ret += '\\';
            ret += prettyEscapes.find(i)->second;
        }
        else if(i < ' ' || _all)
        {
            ret += "\\x";
            ret.push_back("0123456789abcdef"[(uint8_t)i / 16]);
            ret.push_back("0123456789abcdef"[(uint8_t)i % 16]);
        }
        else
            ret.push_back(i);
    }
    ret.push_back('"');
    return ret;
}

int XDag::FromHex(char _i, WhenError _throw)
{
    if(_i >= '0' && _i <= '9')
        return _i - '0';
    if(_i >= 'a' && _i <= 'f')
        return _i - 'a' + 10;
    if(_i >= 'A' && _i <= 'F')
        return _i - 'A' + 10;
    if(_throw == WhenError::Throw)
        BOOST_THROW_EXCEPTION(BadHexCharacter() << errinfo_invalidSymbol(_i));
    else
        return -1;
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
