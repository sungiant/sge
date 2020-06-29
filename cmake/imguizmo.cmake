project (imguizmo)

file (GLOB_RECURSE SOURCES ${G_ROOT_DIR}/contrib/imguizmo/*.cpp)
file (GLOB_RECURSE INCLUDES ${G_ROOT_DIR}/contrib/imguizmo/*.h)

add_library (imguizmo ${SOURCES} ${INCLUDES})

set_target_properties(imguizmo PROPERTIES FOLDER Contrib)

target_link_libraries (imguizmo imgui)