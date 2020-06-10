project (imgui)

file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/contrib/*.cpp)
file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/contrib/*.h)

add_library (imgui ${SOURCES} ${INCLUDES})

set_target_properties(imgui PROPERTIES FOLDER Contrib)
