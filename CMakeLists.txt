cmake_minimum_required(VERSION 3.10)
project(Phoenix)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE MAIN "main.cpp")
file(GLOB_RECURSE CIVET_C "libs/Civet/civetweb.c")
file(GLOB_RECURSE CIVET_INL "libs/Civet/*.inl")
file(GLOB_RECURSE CORE "core/*.cpp")
file(GLOB_RECURSE PAGES "pages/*.cpp")


add_subdirectory(routes)

add_executable(${CMAKE_PROJECT_NAME} ${MAIN} ${CORE} ${CIVET_C} ${CIVET_INL} ${PAGES})
target_link_libraries(${CMAKE_PROJECT_NAME} 
    ws2_32
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE core
    PRIVATE pages
    PRIVATE routes
    PUBLIC libs
)

add_definitions(-DNO_SSL)