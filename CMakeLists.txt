CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(Phoenix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE MAIN "core/core.cpp")
file(GLOB_RECURSE PHOENIX "core/libs/phoenix/*.cpp")
file(GLOB_RECURSE SERVER "core/libs/Civet/civetweb.c")
file(GLOB_RECURSE SERVER_INL "core/libs/Civet/*.inl")
list(APPEND PHOENIX "core/phoenix/engine.cpp")
add_subdirectory(core/handler)

add_executable(${CMAKE_PROJECT_NAME} ${MAIN}  ${PHOENIX}  ${SERVER} ${SERVER_INL})

target_include_directories(${CMAKE_PROJECT_NAME}
  PUBLIC ${CMAKE_SOURCE_DIR}/core/handler/
  PUBLIC ${CMAKE_SOURCE_DIR}/core/libs/
  PUBLIC ${CMAKE_SOURCE_DIR}/core/phoenix/
)

if (WIN32)
    add_definitions(-DNO_SSL)
else()
    add_definitions(-DNO_SSL -DLINUX)
    # Export symbols from the executable so shared libraries can use them
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wl,--export-dynamic")
endif()
