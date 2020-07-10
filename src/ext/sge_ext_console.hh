#pragma once

#include "sge.hh"

// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class console : public runtime::view {
    std::wstring default_channel = L"DEFAULT";

    static const size_t MAX_MSG_LEN = 2048;
    static const size_t MAX_CHN_LEN = 64;

    std::array<char, MAX_MSG_LEN> working_data;
    std::array<wchar_t, MAX_MSG_LEN> working_data_w;

    std::array<wchar_t, MAX_CHN_LEN> working_data_c;

public:

    console (const runtime::api& z) : runtime::view (z, "Console", 0) {}

#if TARGET_WIN32
    void debug   (const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::debug,   default_channel.c_str (), working_data_w.data ()); }
    void info    (const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::info,    default_channel.c_str (), working_data_w.data ()); }
    void warning (const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::warning, default_channel.c_str (), working_data_w.data ()); }
    void error   (const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::error,   default_channel.c_str (), working_data_w.data ()); }

    void debug   (const wchar_t* chn, const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::debug, chn,   working_data_w.data ()); }
    void info    (const wchar_t* chn, const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::info, chn,    working_data_w.data ()); }
    void warning (const wchar_t* chn, const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::warning, chn, working_data_w.data ()); }
    void error   (const wchar_t* chn, const wchar_t* fmt, ...) { va_list args; va_start (args, fmt); _vsnwprintf (working_data_w.data (), working_data_w.size (), fmt, args); va_end (args); sge.tty_log (runtime::log_level::error, chn,   working_data_w.data ()); }
#else
    
    // not sure how to do _vsnwprintf for posix systems...
    
    void debug   (const wchar_t* fmt, ...) { assert (false); }
    void info    (const wchar_t* fmt, ...) { assert (false); }
    void warning (const wchar_t* fmt, ...) { assert (false); }
    void error   (const wchar_t* fmt, ...) { assert (false); }

    void debug   (const wchar_t* chn, const wchar_t* fmt, ...) { assert (false); }
    void info    (const wchar_t* chn, const wchar_t* fmt, ...) { assert (false); }
    void warning (const wchar_t* chn, const wchar_t* fmt, ...) { assert (false); }
    void error   (const wchar_t* chn, const wchar_t* fmt, ...) { assert (false); }

#endif
    
    void debug   (const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data(), MAX_MSG_LEN); sge.tty_log (runtime::log_level::debug, default_channel.c_str (),   working_data_w.data ()); }
    void info    (const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::info, default_channel.c_str (),    working_data_w.data ()); }
    void warning (const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::warning, default_channel.c_str (), working_data_w.data ()); }
    void error   (const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::error, default_channel.c_str (),   working_data_w.data ()); }

    void debug   (const wchar_t* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::debug,   chn, working_data_w.data ()); }
    void info    (const wchar_t* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::info,    chn, working_data_w.data ()); }
    void warning (const wchar_t* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::warning, chn, working_data_w.data ()); }
    void error   (const wchar_t* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::error,   chn, working_data_w.data ()); }

    void debug   (const char* chn, const char* fmt, ...) { mbstowcs (working_data_c.data (), chn, MAX_CHN_LEN); va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::debug,   working_data_c.data(), working_data_w.data ()); }
    void info    (const char* chn, const char* fmt, ...) { mbstowcs (working_data_c.data (), chn, MAX_CHN_LEN); va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::info,    working_data_c.data (), working_data_w.data ()); }
    void warning (const char* chn, const char* fmt, ...) { mbstowcs (working_data_c.data (), chn, MAX_CHN_LEN); va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::warning, working_data_c.data (), working_data_w.data ()); }
    void error   (const char* chn, const char* fmt, ...) { mbstowcs (working_data_c.data (), chn, MAX_CHN_LEN); va_list args; va_start (args, fmt); vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); mbstowcs (working_data_w.data (), working_data.data (), MAX_MSG_LEN); sge.tty_log (runtime::log_level::error,   working_data_c.data (), working_data_w.data ()); }


};

}
