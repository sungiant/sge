#pragma once

#include <string>
#include <sstream>

#define SGE_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))

#define SGE_VERSION SGE_MAKE_VERSION(0, 0, 0)

#define SGE_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define SGE_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define SGE_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

namespace sge {
inline std::string version () {
    std::stringstream ss;
    ss << SGE_VERSION_MAJOR (SGE_VERSION) << "." << SGE_VERSION_MINOR (SGE_VERSION) << "." << SGE_VERSION_PATCH (SGE_VERSION);
    return ss.str();
}
}

