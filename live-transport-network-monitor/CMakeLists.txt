cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(network-monitor)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc")

add_executable(network-monitor ${SOURCES})