#include <memory>
#include <optional>
#include <string>

#include <imgui/imgui.h>
#include <sge.h>

std::unique_ptr<sge::app::configuration> config;
std::unique_ptr<sge::app::content> computation;

void initialise () {
    config = std::make_unique<sge::app::configuration> ();
    
    computation = std::make_unique<sge::app::content>(sge::app::content {
        "vanilla.comp.spv",
        std::optional<sge::app::content::span>(),
        {}
    });
}

void terminate () { config.reset (); }


//--------------------------------------------------------------------------------------------------------------------//
namespace sge::app { // HOOK UP TO SGE
    
void               initialise          ()                              { ::initialise (); }
configuration&     get_configuration   ()                              { return *::config; }
content&           get_content         ()                              { return *::computation; }
void               start               (const api& sge)                {}
void               update              (response& r, const api& sge)   {}
void               debug_ui            (response& r, const api& sge)   {}
void               stop                (const api& sge)                {}
void               terminate           ()                              { ::terminate (); }

}
