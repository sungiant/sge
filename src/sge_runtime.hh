// SGE-RUNTIME
// -------------------------------------
// SGE's low level runtime interface.
// -------------------------------------
// * A thin layer over sge::core to avoid the need for direct user access.
// * SGE systems/views are built on top of this API.
// -------------------------------------

#pragma once

#include <map>
#include <memory>

#include "sge_types.hh"                // todo: remove need for this dependency
#include "sge_math.hh"

// sge::runtime's public interface should not know about the details of sge::core so don't include sge_core.hh in this file.
namespace sge::core { struct engine_state; struct engine_tasks; }

namespace sge::runtime {

enum class system_bool_state {
    fullscreen,                         // run in fullscreen mode?
    imgui,                              // enable imgui
};

enum class system_int_state {
    screenwidth,                        // screen width
    screenheight,                       // screen height
};

enum class system_string_state {
    title,                              // window title
    gpu_name,
};

// the runtime api is a low level interface for interacting with SGE at runtime.
class api {
    const core::engine_state& engine_state;
    core::engine_tasks& engine_tasks;
public:

    api (const core::engine_state&, core::engine_tasks&);

    bool                    system__get_state_bool              (system_bool_state) const;
    int                     system__get_state_int               (system_int_state) const;
    const char*             system__get_state_string            (system_string_state) const;
    bool                    system__did_container_just_change   () const;

    uint32_t                timer__get_fps                      () const;
    float                   timer__get_delta                    () const;
    float                   timer__get_time                     () const;

    void                    input__get_state                    (input_state&) const;



    // The intention is that every non-const function call to this api will be captured
    // and enqueued for later processing.
    void                    system__request_shutdown            ();
    void                    system__toggle_state_bool           (system_bool_state);
    void                    system__set_state_bool              (system_bool_state, bool);
    void                    system__set_state_int               (system_int_state, int);
    void                    system__set_state_string            (system_string_state, const char*);

};


// runtime extensions are managed by the core engine and facilitiate the provision of high level
// functionality based on top of the runtime api.
class extension {
public:
    virtual void update () {};
    virtual void debug_ui () {};
    extension () {};
public:
    virtual ~extension () {};
};

// runtime views have readonly (const) access to the runtime api.
class view : public extension {
protected:
    const api& sge;
public:
    view (const api& z) : sge (z) {}
    virtual ~view () {};
};

// runtime systems have full access to the runtime api.
class system : public extension {
protected:
    api& sge;
public:
    system (api& z) : sge (z) {}
    virtual ~system () {};
};

}
