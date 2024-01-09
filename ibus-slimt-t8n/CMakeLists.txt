set(IBUS_BUS_NAME "org.freedesktop.IBus.slimt")
set(IBUS_ENGINE_NAME "slimt-t8n")
set(IBUS_ENGINE_EXECUTABLE_NAME "ibus-${IBUS_ENGINE_NAME}")
set(IBUS_COMMANDLINE
    "${CMAKE_INSTALL_LIBEXECDIR}/${IBUS_ENGINE_EXECUTABLE_NAME}")
set(IBUS_TEXTDOMAIN "ibus-slimt")
set(IBUS_ICON "${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/bergamot.png")
set(IBUS_LAYOUT "us")
set(IBUS_LANGUAGE "en")

set(IBUS_FULL_COMMANDLINE
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}/${IBUS_ENGINE_EXECUTABLE_NAME} --ibus"
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${IBUS_ENGINE_NAME}.xml.in"
               "${CMAKE_BINARY_DIR}/${IBUS_ENGINE_NAME}.xml")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ibus_config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/ibus_config.h" @ONLY)

add_library(slimt-t8n STATIC engine_compat.cpp slimt_engine.cpp translator.cpp
                             application.cpp)
target_link_libraries(slimt-t8n PUBLIC ${SLIMT_T8N_PRIVATE_LIBS})

target_include_directories(
  slimt-t8n PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
                   $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)

target_include_directories(slimt-t8n PUBLIC ${GLIB2_INCLUDE_DIRS}
                                            ${IBUS_INCLUDE_DIRS})

add_executable(${IBUS_ENGINE_EXECUTABLE_NAME} main.cpp)
target_link_libraries(${IBUS_ENGINE_EXECUTABLE_NAME} PUBLIC slimt-t8n)

add_executable(test test.cpp)
target_link_libraries(test PUBLIC slimt-t8n)
