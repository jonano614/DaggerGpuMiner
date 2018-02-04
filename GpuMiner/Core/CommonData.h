/*
  This file is taken from ethminer project.

  Shared algorithms and data types.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <vector>
#include <unordered_set>
#include <cstring>
#include <string>
#include "Common.h"

namespace XDag
{
    // String conversion functions, mainly to/from hex/nibble/byte representations.

    enum class WhenError
    {
        DontThrow = 0,
        Throw = 1,
    };

    enum class HexPrefix
    {
        DontAdd = 0,
        Add = 1,
    };

    /// Convert a series of bytes to the corresponding string of hex duplets.
    /// @param w specifies the width of the first of the elements. Defaults to two - enough to represent a byte.
    /// @example toHex("A\x69") == "4169"
    template <class T>
    std::string ToHex(T const& data, int w = 2, HexPrefix prefix = HexPrefix::DontAdd)
    {
        std::ostringstream ret;
        unsigned ii = 0;
        for(auto i : data)
        {
            ret << std::hex << std::setfill('0') << std::setw(ii++ ? 2 : w) << (int)(typename std::make_unsigned<decltype(i)>::type)i;
        }
        return (prefix == HexPrefix::Add) ? "0x" + ret.str() : ret.str();
    }

    /// Converts a (printable) ASCII hex character into the correspnding integer value.
    /// @example fromHex('A') == 10 && fromHex('f') == 15 && fromHex('5') == 5
    int FromHex(char i, WhenError _throw);

    inline std::string ToHex(uint64_t n)
    {
        std::ostringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(sizeof(n) * 2) << n;
        return ss.str();
    }

    /// Sets enviromental variable.
    ///
    /// Portable wrapper for setenv / _putenv C library functions.
    bool SetEnv(const char name[], const char value[], bool override = false);
}
