#pragma once

#include "sge.hh"

// INSTRUMENTATION
//--------------------------------------------------------------------------------------------------------------------//

namespace sge::ext {

class console : public runtime::view {
    std::string default_channel = "DEFAULT";

    std::array<char, 2048> working_data;

public:

    console (const runtime::api& z) : runtime::view (z, "Console", 0) {}

    void debug   (const char* msg) const { return sge.tty_debug   (default_channel.c_str (), msg); }
    void info    (const char* msg) const { return sge.tty_info    (default_channel.c_str(), msg); }
    void warning (const char* msg) const { return sge.tty_warning (default_channel.c_str(), msg); }
    void error   (const char* msg) const { return sge.tty_error   (default_channel.c_str(), msg); }

    void debug   (const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_debug   (default_channel.c_str (), working_data.data ()); }
    void info    (const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_info    (default_channel.c_str (), working_data.data ()); }
    void warning (const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_warning (default_channel.c_str (), working_data.data ()); }
    void error   (const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_error   (default_channel.c_str (), working_data.data ()); }

    void debug   (const char* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_debug   (chn, working_data.data ()); }
    void info    (const char* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_info    (chn, working_data.data ()); }
    void warning (const char* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_warning (chn, working_data.data ()); }
    void error   (const char* chn, const char* fmt, ...) { va_list args; va_start (args, fmt); _vsnprintf (working_data.data (), working_data.size (), fmt, args); va_end (args); sge.tty_error   (chn, working_data.data ()); }

};

}
