cmake_minimum_required(VERSION 3.15) #非必须
project(test)
set(CMAKE_CXX_STANDARD 17)

# set(SRC add.cpp sub.cpp mult.cpp div.cpp main.cpp)
# aux_source_directory(${PROJECT_SOURCE_DIR} SRC)
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(EXECUTABLE_OUTPUT_PATH /home/zlf/coding/cpp-netCoding/cmake-test/aa/bb/cc)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(app ${SRC})