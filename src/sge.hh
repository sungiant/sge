// ---------------------------------- //
//   _________ ___________________    //
//  /   _____//  _____/\_   _____/    //
//  \_____  \/   \  ___ |    __)_     //
//  /        \    \_\  \|        \    //
// /_______  /\______  /_______  /    //
//         \/        \/  0.1.0 \/     //
// ---------------------------------- //
// Sungiant's game engine             //
// Copyright (C) 2020, A.J.Pook       //
// https://github.com/sungiant/sge    //
// ---------------------------------- //

#pragma once

// ---------------------------------- //

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
//#include <float.h>

// ---------------------------------- //

#include <memory>
#include <thread>
#include <optional>
#include <variant>
#include <type_traits>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <queue>
#include <array>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <span>
#include <numeric>

// ---------------------------------- //

#if TARGET_WIN32
#include <windows.h>
#undef min
#undef max
#undef near
#undef far
#endif

#if TARGET_LINUX
#include <xcb/xcb.h>
#endif

// ---------------------------------- //

#include <vulkan/vulkan.h>

#if TARGET_MACOSX
#include <MoltenVK/mvk_vulkan.h>
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

// ---------------------------------- //

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

// ---------------------------------- //

#define SGE_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))
#define SGE_VERSION SGE_MAKE_VERSION(0, 1, 0)
#define SGE_VERSION_MAJOR(version) ((uint32_t)(version) >> 22)
#define SGE_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3ff)
#define SGE_VERSION_PATCH(version) ((uint32_t)(version) & 0xfff)

#define ASSERT(x) assert(x);
#define SAFE_DELETE(x) { delete x; x = nullptr; }
#define TRACE(x) { printf (x); }

// ---------------------------------- //

namespace sge {

struct dataspan {
    void* address;
    size_t size;
    bool operator == (const dataspan& s) const { return address == s.address && size == s.size; }
    bool operator != (const dataspan& s) const { return !(*this == s); }
};


}
