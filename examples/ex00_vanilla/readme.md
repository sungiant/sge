# SGE Example 00 - Vanilla

This example renders a gradient on the GPU with compute shaders using SGE.

For SGE it demonstrates:
* how to create a minimal application.

![Screenshot](/docs/ex00.jpg)

Here's how it works, just:

```glsl
#version 450

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba8) uniform writeonly image2D img;

void main() {
    ivec2 dim = imageSize (img);
    vec2 uv = vec2 (gl_GlobalInvocationID.xy) / dim;
    vec4 result = vec4 (vec3 (.5) * (1. - (length(uv - vec2(0.5)) - .2)), 1);
    imageStore (img, ivec2 (gl_GlobalInvocationID.xy), result);
}
```

and this:

```C++
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
        std::optional<sge::dataspan>(),
        {}
    });
}

void terminate () { config.reset (); }

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
```


