project (sge)

file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/src/*.hh ${G_ROOT_DIR}/src/*.h ${G_ROOT_DIR}/src/*.inl)
file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/src/*.cc ${G_ROOT_DIR}/src/*.c)

set (SOURCE_LIST ${INCLUDES} ${SOURCES})

filter_items(SOURCE_LIST "${G_ROOT_DIR}/src/impl_*") # remove platform specific code

################################################################################

if (G_TARGET STREQUAL "WIN32")

add_library (sge ${SOURCE_LIST})

target_include_directories (sge PUBLIC ${G_ROOT_DIR}/src/)

elseif (G_TARGET STREQUAL "MACOSX")

set (MVK_FRAMEWORK $ENV{MOLTEN_VK}/macOS/framework/MoltenVK.framework)

add_library (sge ${SOURCE_LIST} ${MVK_FRAMEWORK})

target_include_directories (sge PUBLIC ${G_ROOT_DIR}/src/)
target_include_directories (sge PUBLIC $ENV{MOLTEN_VK}/include)

target_link_libraries (sge ${MVK_FRAMEWORK})

elseif (G_TARGET STREQUAL "LINUX")
	
add_library (sge ${SOURCE_LIST})

target_include_directories (sge PUBLIC ${G_ROOT_DIR}/src/)

target_link_libraries (sge xcb)

endif ()


if (G_TARGET STREQUAL "WIN32")
add_definitions (-DSGE_VK_USE_CUSTOM_ALLOCATOR)
endif ()

#add_definitions (-DSGE_DEBUG_MODE)
#add_definitions (-DSGE_PROFILING_MODE)

################################################################################

foreach (_source IN ITEMS ${SOURCE_LIST})
    get_filename_component (_source_path "${_source}" PATH)
    string (REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string (REPLACE "/" "\\" _group_path "${_group_path}")
    string (REPLACE "..\\src" "" _group_path "${_group_path}")
    source_group ("${_group_path}" FILES "${_source}")
endforeach ()

target_link_libraries (sge Vulkan::Vulkan imgui imguizmo)
