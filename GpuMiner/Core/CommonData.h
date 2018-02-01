/*
  This file is taken from ethminer project.

  Shared algorithms and data types.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#ifdef __linux__
typedef unsigned char byte;
#endif
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
    /// @param _w specifies the width of the first of the elements. Defaults to two - enough to represent a byte.
    /// @example toHex("A\x69") == "4169"
    template <class T>
    std::string ToHex(T const& _data, int _w = 2, HexPrefix _prefix = HexPrefix::DontAdd)
    {
        std::ostringstream ret;
        unsigned ii = 0;
        for(auto i : _data)
            ret << std::hex << std::setfill('0') << std::setw(ii++ ? 2 : _w) << (int)(typename std::make_unsigned<decltype(i)>::type)i;
        return (_prefix == HexPrefix::Add) ? "0x" + ret.str() : ret.str();
    }

    /// Converts a (printable) ASCII hex character into the correspnding integer value.
    /// @example fromHex('A') == 10 && fromHex('f') == 15 && fromHex('5') == 5
    int FromHex(char _i, WhenError _throw);

    // Big-endian to/from host endian conversion functions.

    /// Converts a templated integer value to the big-endian byte-stream represented on a templated collection.
    /// The size of the collection object will be unchanged. If it is too small, it will not represent the
    /// value properly, if too big then the additional elements will be zeroed out.
    /// @a Out will typically be either std::string or bytes.
    /// @a T will typically by unsigned, u160, u256 or bigint.
    template <class T, class Out>
    inline void ToBigEndian(T _val, Out& o_out)
    {
        static_assert(!std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
        for(auto i = o_out.size(); i != 0; _val >>= 8, i--)
        {
            T v = _val & (T)0xff;
            o_out[i - 1] = (typename Out::value_type)(uint8_t)v;
        }
    }

    /// Converts a big-endian byte-stream represented on a templated collection to a templated integer value.
    /// @a _In will typically be either std::string or bytes.
    /// @a T will typically by unsigned, u160, u256 or bigint.
    template <class T, class _In>
    inline T FromBigEndian(_In const& _bytes)
    {
        T ret = (T)0;
        for(auto i : _bytes)
            ret = (T)((ret << 8) | (byte)(typename std::make_unsigned<typename _In::value_type>::type)i);
        return ret;
    }

    inline std::string ToHex(uint64_t _n)
    {
        std::ostringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(sizeof(_n) * 2) << _n;
        return ss.str();
    }

    // Algorithms for string and string-like collections.

    /// Escapes a string into the C-string representation.
    /// @p _all if true will escape all characters, not just the unprintable ones.
    std::string Escaped(std::string const& _s, bool _all = true);

    // General datatype convenience functions.

    /// Determine bytes required to encode the given integer value. @returns 0 if @a _i is zero.
    template <class T>
    inline unsigned BytesRequired(T _i)
    {
        static_assert(!std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); //bigint does not carry sign bit on shift
        unsigned i = 0;
        for(; _i != 0; ++i, _i >>= 8) {}
        return i;
    }

    /// Sets enviromental variable.
    ///
    /// Portable wrapper for setenv / _putenv C library functions.
    bool SetEnv(const char name[], const char value[], bool override = false);
}
