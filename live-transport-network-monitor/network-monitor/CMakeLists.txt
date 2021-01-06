cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

# Project name
project(network-monitor)

# =============================================================================
# Dependencies: If dependencies are managed with conan, we set the CMake module
# path (where CMake looks for dependencies) to the current build folder.
# =============================================================================
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif ()
find_package(Boost 1.74 REQUIRED COMPONENTS system unit_test_framework)
find_package(OpenSSL REQUIRED)

# =============================================================================
# Enable testing; needs to be called before any other target is defined.
# =============================================================================
enable_testing()

# =============================================================================
# Build the user-defined static library
# =============================================================================
# Our library source files
set(LIBRARY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket_client.cc")

# Make sure we build it as static and call it network-monitor
add_library(network-monitor STATIC ${LIBRARY_SOURCES})

# The compilation flags we need for this library
target_compile_features(network-monitor PUBLIC cxx_std_14)

# The location of the corresponding header files
target_include_directories(network-monitor PUBLIC include)

# The additional libraries this requires
target_link_libraries(network-monitor PUBLIC Boost::Boost OpenSSL::OpenSSL)

# =============================================================================
# Tests
# =============================================================================
# Source files
set(TESTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cc"
                  "${CMAKE_CURRENT_SOURCE_DIR}/tests/websocket_client.cc"
)

# The executable to build
add_executable(network-monitor-tests ${TESTS_SOURCES})

# Build with C++17 and make sure Ninja colourises its output
target_compile_features(network-monitor-tests PRIVATE cxx_std_17)
target_compile_options(network-monitor-tests PUBLIC -fcolor-diagnostics)

# Add the location of the downloaded security certificate (contains the CAs)
target_compile_definitions(
  network-monitor-tests
  PRIVATE TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem"
)

# The libraries the executable needs to link with; this does not include the
# dependencies added transitively through the network-monitor library
target_link_libraries(
  network-monitor-tests PRIVATE network-monitor Boost::Boost
)

# The name of the test this will be built into
add_test(NAME network-monitor-tests
         COMMAND $<TARGET_FILE:network-monitor-tests>
)

# =============================================================================
# This tells CMake to check for a specific output to verify the test outcome.
# When all unit tests pass, Boost.Test prints "No errors detected".
# =============================================================================
set_tests_properties(
  network-monitor-tests PROPERTIES PASS_REGULAR_EXPRESSION
                                   ".*No errors detected"
)
