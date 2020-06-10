list (APPEND EXAMPLES
    ex00_vanilla
    ex01_sinewaves
    ex02_juliaset
    ex03_staticsdf)

foreach(PROJ IN LISTS EXAMPLES)

project (${PROJ})
file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/examples/${PROJ}/*.cpp)
file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/examples/${PROJ}/*.hpp)
file (GLOB_RECURSE RESOURCES ${G_ROOT_DIR}/examples/${PROJ}/*.comp)
set (SOURCE_LIST ${SOURCES} ${INCLUDES} ${RESOURCES})

if (G_TARGET STREQUAL "WIN32")

# add relevant platform specific code
file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_win_*.hpp)
file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_win_*.cpp)
set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_executable (${PROJ} WIN32 ${SOURCE_LIST})

elseif (G_TARGET STREQUAL "MACOSX")

# add relevant platform specific code
file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_osx_*.hpp)
file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_osx_*.cpp ${G_ROOT_DIR}/src/sge_osx_*.mm)
set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_executable (${PROJ} MACOSX_BUNDLE ${SOURCE_LIST})
target_link_libraries (${PROJ} "-framework AppKit")
target_link_libraries (${PROJ} "-framework IOKit")
target_link_libraries (${PROJ} "-framework QuartzCore")
target_link_libraries (${PROJ} "-framework IOSurface")
target_link_libraries (${PROJ} "-framework Metal")
target_link_libraries (${PROJ} "-framework MetalKit")

endif ()

set_target_properties(${PROJ} PROPERTIES
    FOLDER Examples
    RESOURCE ${RESOURCES})

target_link_libraries (${PROJ} sge imgui)

endforeach()

