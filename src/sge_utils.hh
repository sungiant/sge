//
// SGE-UTILS
// ---------------------------------- //
// Stand alone utilities.
// ---------------------------------- //

#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

namespace sge::utils {

inline void get_file_stream (std::vector<uint8_t>& output, const char* path) {
    std::ifstream input (path, std::ios::binary);

    std::vector<char> bytes (
        (std::istreambuf_iterator<char>(input)),
        (std::istreambuf_iterator<char>()));

    assert (bytes.size () > 0);

    output.resize (bytes.size ());

    for (int i = 0; i < bytes.size (); ++i) {
        output[i] = (uint8_t) bytes[i];
    }

    input.close ();
}

}
