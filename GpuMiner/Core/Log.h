/*
   This file is taken from ethminer project.

   The logging subsystem.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <ctime>
#include <chrono>
#include <sstream>
#include "Common.h"
#include "CommonData.h"
#include "Terminal.h"

//namespace boost { namespace asio { namespace ip { template<class T>class basic_endpoint; class tcp; } } }

namespace XDag
{
    /// The null output stream. Used when logging is disabled.
    class NullOutputStream
    {
    public:
        template <class T> NullOutputStream& operator<<(T const&) { return *this; }
    };

    /// A simple log-output function that prints log messages to stdout.
    void SimpleDebugOut(std::string const&);

    /// The logging system's current verbosity.
    extern int g_logVerbosity;

    class ThreadContext
    {
    public:
        ThreadContext(std::string const& _info) { push(_info); }
        ~ThreadContext() { pop(); }

        static void push(std::string const& _n);
        static void pop();
        static std::string join(std::string const& _prior);
    };

    /// Set the current thread's log name.
    void SetThreadName(char const* _n);

    /// Get the current thread's log name.
    std::string GetThreadName();

    /// The default logging channels. Each has an associated verbosity and three-letter prefix (name() ).
    /// Channels should inherit from LogChannel and define name() and verbosity.
    struct LogChannel { static const char* name(); static const int verbosity = 1; static const bool debug = true; };
    struct LeftChannel : public LogChannel { static const char* name(); };
    struct RightChannel : public LogChannel { static const char* name(); };
    struct WarnChannel : public LogChannel { static const char* name(); static const int verbosity = 0; static const bool debug = false; };
    struct NoteChannel : public LogChannel { static const char* name(); static const bool debug = false; };
    struct DebugChannel : public LogChannel { static const char* name(); static const int verbosity = 0; };

    enum class LogTag
    {
        None,
        Url,
        Error,
        Special
    };

    class LogOutputStreamBase
    {
    public:
        LogOutputStreamBase(char const* _id, std::type_info const* _info, unsigned _v, bool _autospacing);

        void Comment(std::string const& _t)
        {
            switch(m_logTag)
            {
            case LogTag::Url: _sstr << EthNavyUnder; break;
            case LogTag::Error: _sstr << EthRedBold; break;
            case LogTag::Special: _sstr << EthWhiteBold; break;
            default:;
            }
            _sstr << _t << EthReset;
            m_logTag = LogTag::None;
        }

        void Append(unsigned long _t) { _sstr << EthBlue << _t << EthReset; }
        void Append(long _t) { _sstr << EthBlue << _t << EthReset; }
        void Append(unsigned int _t) { _sstr << EthBlue << _t << EthReset; }
        void Append(int _t) { _sstr << EthBlue << _t << EthReset; }
        void Append(double _t) { _sstr << EthBlue << _t << EthReset; }
        void Append(std::string const& _t) { _sstr << EthGreen "\"" + _t + "\"" EthReset; }
        template <class T> void Append(std::vector<T> const& _t)
        {
            _sstr << EthWhite "[" EthReset;
            int n = 0;
            for(auto const& i : _t)
            {
                _sstr << (n++ ? EthWhite ", " EthReset : "");
                Append(i);
            }
            _sstr << EthWhite "]" EthReset;
        }
        template <class T> void Append(std::set<T> const& _t)
        {
            _sstr << EthYellow "{" EthReset;
            int n = 0;
            for(auto const& i : _t)
            {
                _sstr << (n++ ? EthYellow ", " EthReset : "");
                Append(i);
            }
            _sstr << EthYellow "}" EthReset;
        }
        template <class T, class U> void Append(std::map<T, U> const& _t)
        {
            _sstr << EthLime "{" EthReset;
            int n = 0;
            for(auto const& i : _t)
            {
                _sstr << (n++ ? EthLime ", " EthReset : "");
                Append(i.first);
                _sstr << (n++ ? EthLime ": " EthReset : "");
                Append(i.second);
            }
            _sstr << EthLime "}" EthReset;
        }
        template <class T> void Append(std::unordered_set<T> const& _t)
        {
            _sstr << EthYellow "{" EthReset;
            int n = 0;
            for(auto const& i : _t)
            {
                _sstr << (n++ ? EthYellow ", " EthReset : "");
                Append(i);
            }
            _sstr << EthYellow "}" EthReset;
        }
        template <class T, class U> void Append(std::unordered_map<T, U> const& _t)
        {
            _sstr << EthLime "{" EthReset;
            int n = 0;
            for(auto const& i : _t)
            {
                _sstr << (n++ ? EthLime ", " EthReset : "");
                Append(i.first);
                _sstr << (n++ ? EthLime ": " EthReset : "");
                Append(i.second);
            }
            _sstr << EthLime "}" EthReset;
        }
        template <class T, class U> void Append(std::pair<T, U> const& _t)
        {
            _sstr << EthPurple "(" EthReset;
            Append(_t.first);
            _sstr << EthPurple ", " EthReset;
            Append(_t.second);
            _sstr << EthPurple ")" EthReset;
        }
        template <class T> void Append(T const& _t)
        {
            _sstr << ToString(_t);
        }

    protected:
        bool m_autospacing = false;
        unsigned m_verbosity = 0;
        std::stringstream _sstr;	///< The accrued log entry.
        LogTag m_logTag = LogTag::None;
    };

    /// Logging class, iostream-like, that can be shifted to.
    template <class Id, bool _AutoSpacing = true>
    class LogOutputStream : LogOutputStreamBase
    {
    public:
        /// Construct a new object.
        /// If _term is true the the prefix info is terminated with a ']' character; if not it ends only with a '|' character.
        LogOutputStream() : LogOutputStreamBase(Id::name(), &typeid(Id), Id::verbosity, _AutoSpacing) {}

        /// Destructor. Posts the accrued log entry to the g_logPost function.
        ~LogOutputStream() { if(Id::verbosity <= g_logVerbosity) SimpleDebugOut(_sstr.str()); }

        LogOutputStream& operator<<(std::string const& _t) { if(Id::verbosity <= g_logVerbosity) { if(_AutoSpacing && _sstr.str().size() && _sstr.str().back() != ' ') _sstr << " "; Comment(_t); } return *this; }

        LogOutputStream& operator<<(LogTag _t) { m_logTag = _t; return *this; }

        /// Shift arbitrary data to the log. Spaces will be added between items as required.
        template <class T> LogOutputStream& operator<<(T const& _t) { if(Id::verbosity <= g_logVerbosity) { if(_AutoSpacing && _sstr.str().size() && _sstr.str().back() != ' ') _sstr << " "; Append(_t); } return *this; }
    };

// Kill all logs when when NLOG is defined.
#if NLOG
#define clog(X) nlog(X)
#define cslog(X) nslog(X)
#else
#if NDEBUG
#define clog(X) if (X::debug) {} else dev::LogOutputStream<X, true>()
#define cslog(X) if (X::debug) {} else dev::LogOutputStream<X, false>()
#else
#define clog(X) XDag::LogOutputStream<X, true>()
#define cslog(X) XDag::LogOutputStream<X, false>()
#endif
#endif

// Simple cout-like stream objects for accessing common log channels.
// Dirties the global namespace, but oh so convenient...
#define cdebug clog(XDag::DebugChannel)
#define cnote clog(XDag::NoteChannel)
#define cwarn clog(XDag::WarnChannel)

// Null stream-like objects.
#define ndebug if (true) {} else XDag::NullOutputStream()
#define nlog(X) if (true) {} else XDag::NullOutputStream()
#define nslog(X) if (true) {} else XDag::NullOutputStream()

}
