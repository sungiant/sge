<p align="center">
  <br/>
  <img src="docs/logo.png" width="80px"/>
  <br/>
  SGE
  <br/>
  simple graphics engine
  <br/>
  <br/>|
    <a href="#features">Features</a> |
    <a href="#examples">Examples</a> |
    <a href="#releases">Releases</a> |
  <br/>|
    <a href="#roadmap">Roadmap</a> |
    <a href="#getting-started">Getting started</a>
  <br/>
  <br/>
  <br/>
  <a href="https://travis-ci.org/sungiant/sge">
    <img src="https://api.travis-ci.org/sungiant/sge.svg?branch=master"/>
  </a>
  <a href="https://github.com/sungiant/sge/issues">
    <img alt="Issues" src="https://img.shields.io/github/issues-raw/sungiant/sge.svg"/>
  </a>
  <img alt="License" src="https://img.shields.io/github/license/sungiant/sge"/>
  <br/>
</p>

## Features

SGE has a minimal feature set - you could think of it as being something like [ShaderToy](https://www.shadertoy.com) running against Vulkan for desktop environments with direct control in C++ over shader input data.

- [x] Easily author compute shader based rendering demos.
- [x] Configurable input data: push constants, and uniform buffers.
- [x] Windows support (Vulkan)
- [x] OSX support (Metal via MoltenVK)
- [x] Cross platform input device support: keyboard, mouse, gamepad.
- [x] Cross platform desktop window management: fullscreen, resize, title, console, dpi.
- [x] Easy to access and to hook into ImGui.
- [x] Built in debugging overlays.
- [x] Modular design.
- [x] Minimal codebase, ~6000 lines total (including platform code).
- [x] Almost no dependencies (OS, Vulkan, ImGui).

## Rationale

* Experiment with Vulkan.

## Examples

<table>
  <tr>
    <td><a href="/examples/ex00_vanilla"><img src="/docs/ex00.jpg" width="420px"/></a></td>
    <td><a href="/examples/ex01_sinewaves"><img src="/docs/ex01.jpg" width="420px"/></a></td>
  </tr>
  <tr>
    <td><a href="/examples/ex02_juliaset"><img src="/docs/ex02.jpg" width="420px"/></a></td>
    <td><a href="/examples/ex03_staticsdf"><img src="/docs/ex03.jpg" width="420px"/></a></td>
  </tr>
  <tr>
    <td><a href="/examples/ex04_mandlebulb"><img src="/docs/ex04.jpg" width="420px"/></a></td>
    <td><a href="/examples/ex05_dynamicsdf"><img src="/docs/ex05.jpg" width="420px"/></a></td>
  </tr>
</table>

## Releases

### Current

* **v0.0.0** - Pre-release (unpatched, dev branch, work-in-progress)

### Upcoming

* **v0.1.0** - Initial release

## Roadmap

### 0.1.0 - Remaining tasks

- [ ] Intelligent Vulkan queue selection.
- [x] Configurable storage buffers.
- [ ] API stabilisation - `sge::runtime` to be independent from `sge::core`.
- [ ] OSX gamepad rewrite.
- [ ] Improve and make configurable SGE built-in ImGui windows.
- [ ] Improve graphics context refresh logic.
- [ ] Fix issue with ImGui not refreshing quickly when window is resized.
- [ ] Review codebase and simplify where needed, resolve inconsistencies.
- [x] CI integration.

### 0.2.0

- [ ] Compute shader input/output textures to be user configurable (formats selection, depth write option).
- [ ] User shape/model rendering support (allow amalgamation of 3D primitives into compute generated scenes).
- [ ] User control over multiple parallel compute pipelines.
- [ ] Multi-threaded renderer.

### 0.X.0

- [ ] Runtime shader compilation via glslang runtime integration.
- [ ] Qt host implementation.
- [ ] Linux host implementation.

## Getting started

### Prerequisite software

* [CMake](https://cmake.org)
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

### Environment setup: Windows

* It may be nessasary to update `gen.py` to specify the specific version of Visual Studio being targetted.


Example [PowerShell](https://docs.microsoft.com/en-us/powershell/scripting/overview?view=powershell-7) configuration:

```
$env:VULKAN_SDK="C:\VulkanSDK\1.2.131.2"
```

### Environment setup: OSX

Example [Fish](https://fishshell.com/) configuration:

```
set -x VULKAN_SDK_PATH /Users/sungiant/sdks/vulkansdk-macos-1.2.131.1
set -x VULKAN_SDK $VULKAN_SDK_PATH/macOS
set -x VK_LAYER_PATH $VULKAN_SDK/etc/vulkan/explicit_layer.d
set -x VK_ICD_FILENAMES $VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json
set -x MOLTEN_VK $VULKAN_SDK_PATH/MoltenVK/
set -x MVK_CONFIG_LOG_LEVEL 2
set -x MVK_CONFIG_TRACE_VULKAN_CALLS 0
set -x MVK_ALLOW_METAL_FENCES 1
set -x MVK_ALLOW_METAL_EVENTS 1
set -gx DYLD_LIBRARY_PATH $DYLD_LIBRARY_PATH $VULKAN_SDK/lib
set -gx PATH $PATH $VULKAN_SDK/bin
```

#### Notes

* When running via XCode Vulkan calls can fail due to environment variables being inaccessible; opening XCode with `open -a xcode` can fix the issue.

### Building

To build SGE and it's associated examples:

* Compile the shaders: `python3 res.py`
* Generate IDE project files: `python3 gen.py`
* Open the project in your IDE.  Build.  Run an example.

## Alternatives

* [ShaderToy](https://www.shadertoy.com)
* [Bonzomatic](https://github.com/Gargaj/Bonzomatic)
* [Taichi](https://github.com/taichi-dev/taichi)

## License

SGE is licensed under the **[MIT License](/license)**; you may not use this software except in compliance with the License.



