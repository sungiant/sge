//
// SGE-UTILS
// ---------------------------------- //
// Stand alone utilities.
// ---------------------------------- //

#pragma once

#include "sge.hh"

namespace sge::utils {

inline void get_file_stream (std::vector<uint8_t>& output, const char* path) {
    std::ifstream input (path, std::ios::binary);
    std::vector<char> bytes ((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
    assert (bytes.size () > 0);
    output.resize (bytes.size ());
    for (int i = 0; i < bytes.size (); ++i) {
        output[i] = (uint8_t) bytes[i];
    }
    input.close ();
}

template <typename T>
inline bool contains_value (std::vector<std::optional<T>> xs) {
    return std::find_if (xs.begin (), xs.end (), [](std::optional<T> x) { return x.has_value ();  }) != xs.end ();
}

template <typename T>
inline bool contains (std::unordered_set<T> xs, T value) { return xs.find (value) != xs.end (); }
    
template <typename T> int sign(T val) { return (T(0) < val) - (val < T(0)); }

inline bool    get_flag_at_index (const uint32_t flags, const uint32_t index) { assert (index < 31); return (flags >> index) & 1u; }
inline void toggle_flag_at_index (uint32_t& flags, const uint32_t index) { assert (index < 31); flags = flags ^ (1u << index); }
inline void    set_flag_at_index (uint32_t& flags, const uint32_t index, const bool value) { assert (index < 31); if (get_flag_at_index (flags, index) != value) toggle_flag_at_index (flags, index); }

inline bool    get_flag_at_mask (const uint32_t flags, const uint32_t mask) { return flags & mask; }
inline void toggle_flag_at_mask (uint32_t& flags, const uint32_t mask) { flags = flags ^ mask;}
inline void    set_flag_at_mask (uint32_t& flags, const uint32_t mask, const bool value) { if (get_flag_at_mask (flags, mask) != value) toggle_flag_at_mask (flags, mask); }

}
