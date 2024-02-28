cmake_minimum_required(VERSION 3.16)
project(Gtest)

set(CMAKE_CXX_STANDARD 17)

set(link_lib glog pthread gtest)

aux_source_directory(./ TESTCPPSRC)

add_executable(Gtest ${TESTCPPSRC})

target_link_libraries(Gtest ${link_lib})