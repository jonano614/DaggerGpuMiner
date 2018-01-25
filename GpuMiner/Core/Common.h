/*
   This file is taken from ethminer project.
*/
/* 
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <functional>
#include <string>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>

namespace XDag
{
    /// Converts arbitrary value to string representation using std::stringstream.
    template <class _T>
    std::string ToString(_T const& _t)
    {
        std::ostringstream o;
        o << _t;
        return o.str();
    }
}
