project (sge)

file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/src/*.hpp ${G_ROOT_DIR}/src/*.h ${G_ROOT_DIR}/src/*.inl)
file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/src/*.cpp ${G_ROOT_DIR}/src/*.c)

set (SOURCE_LIST ${INCLUDES} ${SOURCES})

# remove platform specific code
filter_items(SOURCE_LIST "${G_ROOT_DIR}/src/sge_win_.*")
filter_items(SOURCE_LIST "${G_ROOT_DIR}/src/sge_osx_.*")

################################################################################

if (G_TARGET STREQUAL "WIN32")

# add relevant platform specific code
#file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_win32_*.hpp)
#file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_win32_*.cpp)
#set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

add_library (sge ${SOURCE_LIST})

target_include_directories (sge PUBLIC ${G_ROOT_DIR}/src/)

elseif (G_TARGET STREQUAL "MACOSX")

# add relevant platform specific code
#file (GLOB_RECURSE PLATFORM_INCLUDES ${G_ROOT_DIR}/src/sge_osx_*.hpp)
#file (GLOB_RECURSE PLATFORM_SOURCES ${G_ROOT_DIR}/src/sge_osx_*.cpp ${G_ROOT_DIR}/src/sge_osx_*.mm)
#set (SOURCE_LIST ${SOURCE_LIST} ${PLATFORM_INCLUDES} ${PLATFORM_SOURCES})

set (MVK_FRAMEWORK $ENV{MOLTEN_VK}/macOS/framework/MoltenVK.framework)

add_library (sge ${SOURCE_LIST} ${MVK_FRAMEWORK})

target_include_directories (sge PUBLIC ${G_ROOT_DIR}/src/)
target_include_directories (sge PUBLIC $ENV{MOLTEN_VK}/include)

target_link_libraries (sge ${MVK_FRAMEWORK})

endif ()

add_definitions (-DSGE_VK_USE_CUSTOM_ALLOCATOR)
add_definitions (-DSGE_DEBUG_MODE)

################################################################################

foreach (_source IN ITEMS ${SOURCE_LIST})
    get_filename_component (_source_path "${_source}" PATH)
    string (REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string (REPLACE "/" "\\" _group_path "${_group_path}")
    string (REPLACE "..\\src" "" _group_path "${_group_path}")
    source_group ("${_group_path}" FILES "${_source}")
endforeach ()


target_link_libraries (sge Vulkan::Vulkan imgui)
