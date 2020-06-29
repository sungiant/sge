project (imgui)

file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/contrib/imgui/*.cpp)
file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/contrib/imgui/*.h)

add_library (imgui ${SOURCES} ${INCLUDES})

set_target_properties(imgui PROPERTIES FOLDER Contrib)
