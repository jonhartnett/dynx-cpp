cmake_minimum_required(VERSION 3.15)
project(dynx VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)

set(INSTALL_INCLUDE_DIR include/dynx-${dynx_VERSION} CACHE PATH "Installation directory for headers")
set(INSTALL_LIB_DIR lib/dynx-${dynx_VERSION} CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin/dynx-${dynx_VERSION} CACHE PATH "Installation directory for executables")

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS src/*)
file(GLOB_RECURSE INCLUDES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS include/*)
file(GLOB_RECURSE TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS test/*)

add_library(dynx SHARED ${SOURCES} ${INCLUDES})
target_include_directories(dynx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

add_executable(dynx-tests ${TEST_SOURCES})
target_include_directories(dynx-tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(dynx-tests dynx)

install(DIRECTORY include/ DESTINATION ${INSTALL_INCLUDE_DIR})

install(TARGETS dynx EXPORT dynx DESTINATION ${INSTALL_LIB_DIR}/${CMAKE_BUILD_TYPE})

file(GLOB_RECURSE cmake_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/cmake CONFIGURE_DEPENDS cmake/*)
configure_file(dynx-config-version.cmake ${CMAKE_CURRENT_BINARY_DIR}/dynx-config-version.cmake @ONLY)
install(FILES dynx-config.cmake DESTINATION ${INSTALL_LIB_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dynx-config-version.cmake DESTINATION ${INSTALL_LIB_DIR})

install(EXPORT dynx DESTINATION ${INSTALL_LIB_DIR})
