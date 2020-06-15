cmake_minimum_required (VERSION 3.10.2)

project (bootstrap)

# Remove strings matching given regular expression from a list.
# @param(in,out) aItems Reference of a list variable to filter.
# @param aRegEx Value of regular expression to match.
function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

# QUERY DEVELOPMENT PLATFORM
if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    set (G_DEV_ENV WINDOWS)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set (G_DEV_ENV DARWIN)
endif ()


if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set (G_DEV_ENV LINUX)
endif ()

# Pick suitable target platform
################################################################################
if (G_DEV_ENV STREQUAL WINDOWS)
    add_definitions (-DTARGET_WIN32)
    add_definitions (-DWIN32_LEAN_AND_MEAN)
    set (G_TARGET WIN32)
elseif (G_DEV_ENV STREQUAL DARWIN)
    add_definitions (-DTARGET_MACOSX)
    set (G_TARGET MACOSX)
elseif (G_DEV_ENV STREQUAL LINUX)
    add_definitions (-DTARGET_LINUX)
    set (G_TARGET LINUX)
endif ()


# Extra setup for platform based on target variant
################################################################################

if (G_DEV_ENV STREQUAL WINDOWS)
    add_definitions (-DVK_USE_PLATFORM_WIN32_KHR)
    set (CMAKE_CXX_STANDARD 20)
elseif (G_DEV_ENV STREQUAL DARWIN)
    add_definitions (-DVK_USE_PLATFORM_MVK_KHR)
    add_definitions (-DVK_USE_PLATFORM_MACOS_MVK)
    set (CMAKE_CXX_STANDARD 17)
elseif (G_DEV_ENV STREQUAL LINUX)
    add_definitions (-DVK_USE_PLATFORM_WAYLAND_KHR)
    set (CMAKE_CXX_STANDARD 20)
endif ()

################################################################################
set (CMAKE_CXX_STANDARD_REQUIRED ON)

message (STATUS "$VULKAN_SDK: " $ENV{VULKAN_SDK})
message (STATUS "$VK_SDK_PATH: " $ENV{VK_SDK_PATH})

if (G_DEV_ENV STREQUAL LINUX)
    list(APPEND CMAKE_MODULE_PATH $ENV{VK_SDK_PATH}samples/cmake/)
endif ()

find_package (vulkan REQUIRED)

message (STATUS ">> Vulkan_FOUND:    ${Vulkan_FOUND}")
message (STATUS ">> Vulkan_INCLUDE_DIRS:    ${Vulkan_INCLUDE_DIRS}")
message (STATUS ">> Vulkan_LIBRARIES      :    ${Vulkan_LIBRARIES}")
################################################################################


set (G_ROOT_DIR ../)

message ("G_ROOT_DIR: ${G_ROOT_DIR}")

include_directories (PUBLIC ${G_ROOT_DIR}/contrib/)

include (imgui.cmake)
include (sge.cmake)
include (examples.cmake)

project (sge)
