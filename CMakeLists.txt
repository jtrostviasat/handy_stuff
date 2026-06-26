cmake_minimum_required(VERSION 3.15)
project(handy
    VERSION 0.1.0
    DESCRIPTION "Ergonomic helpers for C99: logging, debugging, correctness, handle-based collections"
    LANGUAGES C)

option(HANDY_BUILD_TESTS    "Build handy tests"    ON)
option(HANDY_BUILD_EXAMPLES "Build handy examples" OFF)
option(HANDY_STUFF_DEBUG    "Compile debug logging paths"          OFF)
option(HANDY_RUNTIME_MEMSAFE "Compile runtime assertion paths"     OFF)

add_library(handy STATIC
    src/handy_log.c
)
add_library(handy::handy ALIAS handy)

target_include_directories(handy
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(handy PUBLIC c_std_99)

if(HANDY_STUFF_DEBUG)
    target_compile_definitions(handy PUBLIC HANDY_STUFF_DEBUG=1)
endif()
if(HANDY_RUNTIME_MEMSAFE)
    target_compile_definitions(handy PUBLIC RUNTIME_MEMSAFE=1)
endif()

if(MSVC)
    target_compile_options(handy PRIVATE /W4)
else()
    target_compile_options(handy PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)
install(TARGETS handy EXPORT handyTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/handy DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT handyTargets
    FILE handyConfig.cmake
    NAMESPACE handy::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/handy)

if(HANDY_BUILD_TESTS)
    enable_testing()
    add_executable(handy_test_basics tests/test_basics.c)
    target_link_libraries(handy_test_basics PRIVATE handy)
    add_test(NAME handy_test_basics COMMAND handy_test_basics)
endif()

if(HANDY_BUILD_EXAMPLES)
    add_executable(example_handle_array examples/example_handle_array.c)
    target_link_libraries(example_handle_array PRIVATE handy)
endif()
