list (APPEND EXAMPLES
    ex00_vanilla
    ex01_sinewaves
    ex02_juliaset
    ex03_raymarching
    ex04_mandlebulb
    ex05_csg
    ex06_raytracing)

foreach(PROJ IN LISTS EXAMPLES)

project (${PROJ})
file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/examples/${PROJ}/*.cc ${G_ROOT_DIR}/examples/ex_common/*.cc)
file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/examples/${PROJ}/*.hh ${G_ROOT_DIR}/examples/ex_common/*.hh)
file (GLOB_RECURSE RESOURCES ${G_ROOT_DIR}/examples/${PROJ}/*.comp)
set (SOURCE_LIST ${SOURCES} ${INCLUDES} ${RESOURCES})

if (G_TARGET STREQUAL "WIN32")

# add relevant platform specific code
file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_impl_win*.hh)
file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_impl_win*.cc)
set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_executable (${PROJ} WIN32 ${SOURCE_LIST})

elseif (G_TARGET STREQUAL "MACOSX")

# add relevant platform specific code
file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_impl_osx*.hh)
file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_impl_osx*.cc ${G_ROOT_DIR}/src/sge_impl_osx*.mm)
set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_executable (${PROJ} MACOSX_BUNDLE ${SOURCE_LIST})
target_link_libraries (${PROJ} "-framework AppKit")
target_link_libraries (${PROJ} "-framework IOKit")
target_link_libraries (${PROJ} "-framework QuartzCore")
target_link_libraries (${PROJ} "-framework IOSurface")
target_link_libraries (${PROJ} "-framework Metal")
target_link_libraries (${PROJ} "-framework MetalKit")


elseif (G_TARGET STREQUAL "LINUX")

file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_impl_linux*.hh)
file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_impl_linux*.cc)
set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_executable (${PROJ} ${SOURCE_LIST})

endif ()

set_target_properties(${PROJ} PROPERTIES
    FOLDER Examples
    RESOURCE ${RESOURCES})

target_link_libraries (${PROJ} sge imgui)

################################################################################

foreach (_source IN ITEMS ${SOURCE_LIST})
    get_filename_component (_source_path "${_source}" PATH)
    string (REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string (REPLACE "/" "\\" _group_path "${_group_path}")
    string (REPLACE "..\\examples\\${PROJ}" "" _group_path "${_group_path}")
    source_group ("${_group_path}" FILES "${_source}")
endforeach ()

endforeach()

