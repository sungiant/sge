#include <sge_app.hh>

sge::app::configuration config = {};
sge::app::content computation = {};
sge::app::extensions extensions = {};

void initialise () {
    computation.shader_path = "vanilla.comp.spv";
}

void terminate () {}

//--------------------------------------------------------------------------------------------------------------------//
namespace sge::app { // HOOK UP TO SGE

void               initialise          ()                              { ::initialise (); }
configuration&     get_configuration   ()                              { return ::config; }
content&           get_content         ()                              { return ::computation; }
extensions&        get_extensions      ()                              { return ::extensions; }
void               start               (const api& sge)                {}
void               update              (response& r, const api& sge)   {}
void               debug_ui            (response& r, const api& sge)   {}
void               stop                (const api& sge)                {}
void               terminate           ()                              { ::terminate (); }

}
