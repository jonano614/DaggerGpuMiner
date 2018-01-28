/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <exception>
#include <string>
#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include "CommonData.h"

namespace XDag
{
    /// Base class for all exceptions.
    struct Exception : virtual std::exception, virtual boost::exception
    {
        Exception(std::string _message = std::string()) : m_message(std::move(_message)) {}
        const char* what() const noexcept override { return m_message.empty() ? std::exception::what() : m_message.c_str(); }

    private:
        std::string m_message;
    };

#define DEV_SIMPLE_EXCEPTION(X) struct X: virtual Exception { const char* what() const noexcept override { return #X; } }

    /// Base class for all RLP exceptions.
    struct RLPException : virtual Exception { RLPException(std::string _message = std::string()) : Exception(_message) {} };
#define DEV_SIMPLE_EXCEPTION_RLP(X) struct X: virtual RLPException { const char* what() const noexcept override { return #X; } }

    DEV_SIMPLE_EXCEPTION_RLP(BadCast);
    DEV_SIMPLE_EXCEPTION_RLP(BadRLP);
    DEV_SIMPLE_EXCEPTION_RLP(OversizeRLP);
    DEV_SIMPLE_EXCEPTION_RLP(UndersizeRLP);

    DEV_SIMPLE_EXCEPTION(BadHexCharacter);

    struct ExternalFunctionFailure : virtual Exception { public: ExternalFunctionFailure(std::string _f) : Exception("Function " + _f + "() failed.") {} };

    // error information to be added to exceptions
    using errinfo_invalidSymbol = boost::error_info<struct tag_invalidSymbol, char>;
    using errinfo_comment = boost::error_info<struct tag_comment, std::string>;
}
