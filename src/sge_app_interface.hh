#pragma once

#include "sge.hh"

#include "imgui_ext.hh"

// SGE-APP
// ---------------------------------- //
// The user interface into the engine.
// ---------------------------------- //
// ! Contains unimplemented functions which must be implemented by the user's application.

// Indicates that the function, implemented in user application space, will be called from SGE code.
#define SGE

// foward declarations
namespace sge::runtime { struct api; class extension; class view; class system; }
namespace sge::app { struct api; }

namespace sge::app {

// An SGE app can be customised on initialisation with the settings in this stucture.
struct configuration {
    std::string app_name = "SGE";
    int app_width = 640;
    int app_height = 360;
    bool enable_console = false;
    bool ignore_os_dpi_scaling = true;

    // todo, this shouldn't live here.
    int adjusted_app_width () const { return app_width; }
    int adjusted_app_height () const { return app_height + imgui::ext::guess_main_menu_bar_height(); }
};

// The content of an SGE app is a computation defined by and app and managed by the engine.
// This structure is a collection of the information needed by the engine in order to managed an
// app's computation.
// Compute shaders must be of the form:
//  |#version 450
//  |layout (local_size_x = 16, local_size_y = 16) in;
//  |layout (binding = 0, rgba8) uniform writeonly image2D output;
//  |void main () {
//  |    imageStore (output, ivec2 (gl_GlobalInvocationID.xy), vec4 (0, 0, 0, 1));
//  |}
// Where the output format is currently fixed in the engine to be rgba8 - more flexibility
// to be added in due course, i.e. optional depth buffer output and alternative formats.
struct content {
    std::string shader_path = "";
    std::optional<dataspan> push_constants = {};
    std::vector<dataspan> uniforms = {};
    std::vector<dataspan> blobs = {}; // todo: change to pair<dataspan, size_t> and make it possible to know the maximum size a blob could be over the full course of the app so we can allocate it on the gpu upfront.  right now when the user changes blob size at runtime the whole sbo is deallocated and reallocated to accomodate.
};

struct extensions {
    typedef std::function<runtime::view*(const runtime::api&)> create_view_fn;
    typedef std::function<runtime::system*(runtime::api&)> create_system_fn;

    std::vector<std::pair<size_t, create_view_fn>> views;
    std::vector<std::pair<size_t, create_system_fn>> systems;
};


// TODO: Replace - see notes in sge_runtime.hh
struct response {
    bool request_shutdown;
    bool push_constants_changed;
    std::vector<bool> uniform_changes;
    std::vector<std::optional<dataspan>> blob_changes;
    response (int usz, int bsz): uniform_changes (usz), blob_changes (bsz) {}
};


//----------------------------------------------------------------------------//

namespace internal {

    api* create_user_api (sge::runtime::api&);
    void delete_user_api (api*);
    extensions& get_standard_extensions (); // these are always enabled and the user api provides easy access to them

    //void debug_ui_menu () {};
    //void debug_ui () {};
}


//----------------------------------------------------------------------------//

// Called by the engine at ealiest point possible - use this to allocate.
SGE void initialise ();

// An app must be able to provide the engine with its desired configuration.
SGE configuration& get_configuration ();

// An app must be able to provide the engine with the details of its content.
SGE content& get_content ();

// An app must provide the engine with the details needed to create any user defined extensions it wants.
SGE extensions& get_extensions ();

// Called by the engine once it has been set up, and once only.
SGE void start (const api&);

// Called by the engine each frame.
// - The user_response structure enabled interaction with the running engine.
// - The user_interface structure allows readonly access to the engine.
SGE void update (response&, const api&);

// Called by the engine each frame - use this for ImGUI.
SGE void debug_ui (response&, const api&);

// Called by the engine once it has been shut down, and once only.
SGE void stop (const api&);

// Called by the engine at latest point possible - use this to deallocate.
SGE void terminate ();

}
