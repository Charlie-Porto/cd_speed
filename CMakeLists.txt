cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
#add_compile_options(-g)

set(PROJECT cd-speed)
project(${PROJECT})

set(SRC ${CMAKE_SOURCE_DIR}/src)

add_executable(${PROJECT}
    ${SRC}/main.cpp
    ${SRC}/hf.cpp)

target_include_directories(${PROJECT} PRIVATE
    ${CMAKE_SOURCE_DIR}/externals/json/include)

install(TARGETS ${PROJECT}
    RUNTIME DESTINATION bin)
