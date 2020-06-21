// SGE-CORE
// ---------------------------------- //
// The internals of the engine, not
// intended for broad direct access.
// ---------------------------------- //

#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <map>
#include <optional>
#include <utility>
#include <array>
#include <functional>
#include <chrono>
#include <cassert>

#if TARGET_WIN32
#include <windows.h>
#endif

#if TARGET_LINUX
#include <xcb/xcb.h>
#endif

#include <imgui/imgui.h>

#include "sge_types.hh"
#include "sge_vk.hh"
#include "sge_runtime.hh"
#include "sge_app.hh"

namespace sge::core {

struct container_state {
    bool is_resizing;
    int current_width;
    int current_height;
};

typedef std::function <void (void)>         void_fn;
typedef std::function <void (const char*)>  string_fn;
typedef std::function <void (bool)>         bool_fn;
typedef std::function <void (int, int)>     point_fn;

// collection of copided information about the host
// (the host is the authority on this data).
struct host_state {

    std::optional<string_fn>                set_window_title_fn;
    std::optional<bool_fn>                  set_window_fullscreen_fn;
    std::optional<point_fn>                 set_window_size_fn;
    std::optional<void_fn>                  shutdown_request_fn;

    std::string                             window_title;
    bool                                    is_fullscreen;
    bool                                    container_just_changed;
};

struct configuration_state {
    const float defaultClearColor [4] = { 0.025f, 0.025f, 0.025f, 1.0f };
};

struct platform_state {
#if TARGET_WIN32
    HINSTANCE hinst;
    HWND hwnd;
#elif TARGET_MACOSX
    void* view;
#elif TARGET_LINUX
    xcb_connection_t* connection;
    xcb_window_t* window;
#endif
};

struct instrumentation_state {
    float frameTimer = 1.0f;
    float totalTimer = 0.0f;
    uint32_t frameCounter = 0;
    uint32_t lastFPS = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp;
};


typedef struct vk::vk graphics_state;

struct engine_state {

    // engine input - replaced each frame by copied provided by the platform layer
    container_state container;
    input_state input;

    // engine constant data
    configuration_state configuration;
    platform_state platform;

    // engine state
    host_state host;
    instrumentation_state instrumentation;
    graphics_state graphics;

};

// Stuff for the engine to do when it gets round to it.
// Every user interaction with the engine at runtime should be here.
struct engine_tasks {
    std::optional<bool>                 change_imgui_enabled;
    std::optional<bool>                 change_fullscreen_enabled;
    std::optional<std::string>          change_window_title;
    std::optional<int>                  change_screen_width;
    std::optional<int>                  change_screen_height;
    std::optional<std::monostate>       shutdown_request;
};

//----------------------------------------------------------------------------------------------------------------//

class engine {

    std::unique_ptr<engine_state>                       engine_state;
    std::unique_ptr<engine_tasks>                       engine_tasks;
    std::unique_ptr<runtime::api>                       engine_api;
    std::vector<std::unique_ptr<runtime::extension>>    engine_extensions = {}; // TODO: support typed extension access.
    std::unique_ptr<app::response>                      user_response;
    std::unique_ptr<app::api>                           user_api;
    std::vector<std::function<void ()>>                 debug_fns = {};

public:
    engine ();
    ~engine ();

    void register_set_window_title_callback (const string_fn& z) { engine_state->host.set_window_title_fn = z; }
    void register_set_window_fullscreen_callback (const bool_fn& z) { engine_state->host.set_window_fullscreen_fn = z; }
    void register_set_window_size_callback (const point_fn& z) { engine_state->host.set_window_size_fn = z; }
    void register_shutdown_request_callback (const void_fn& z) { engine_state->host.shutdown_request_fn = z; }

    void setup (
#if TARGET_WIN32
        HINSTANCE,
        HWND
#elif TARGET_MACOSX
        void*
#elif TARGET_LINUX
        xcb_connection_t*,
        xcb_window_t*
#endif
    );

    void start ();
    void update (container_state&, input_state&);
    void stop ();

    void shutdown ();
};

}
