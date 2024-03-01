cmake_minimum_required(VERSION 3.17)
project(test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -fno-elide-constructors)
set(link_lib glog pthread gtest)

aux_source_directory(./test/ CPPSRC)

add_executable(tests ${CPPSRC})

target_link_libraries(tests ${link_lib})