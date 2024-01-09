cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(CMakeDependentOption)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW) # CMake 3.12
endif()
project(ibus-slimt-t8n CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GetVersionFromFile)
message(STATUS "Project name: ${PROJECT_NAME}")
message(STATUS "Project version: ${PROJECT_VERSION_STRING_FULL}")

set(AUTHOR "Jerin Philip <jerinphilip@live.in>")
set(AUTHOR_XML_ESCAPED "Jerin Philip &lt;jerinphilip@live.in&gt;")
set(PROJECT_SHORTNAME "ibus-slimt-t8n")
set(PROJECT_LONGNAME "slimt-t8n")
set(PROJECT_DESCRIPTION
    "slimt-t8n provides client-side translation on the local *nix machine.")
set(PROJECT_VERSION ${PROJECT_VERSION_STRING})
set(PROJECT_HOMEPAGE "https://github.com/jerinphilip/ibus-slimt-t8n")
set(PROJECT_LICENSE "GPL")

find_package(yaml-cpp REQUIRED)
find_package(slimt REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(IBUS REQUIRED ibus-1.0)

if(NOT TARGET yaml-cpp::yaml-cpp)
  add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()

set(SLIMT_T8N_PRIVATE_LIBS slimt::slimt-shared yaml-cpp::yaml-cpp
                           ${GLIB2_LIBRARIES} ${IBUS_LIBRARIES})

include(GNUInstallDirs)

add_subdirectory(ibus-slimt-t8n)

install(TARGETS ibus-slimt-t8n RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CMAKE_BINARY_DIR}/slimt-t8n.xml
        DESTINATION /usr/share/ibus/component)
install(FILES ${CMAKE_SOURCE_DIR}/assets/bergamot.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
