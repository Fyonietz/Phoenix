file(GLOB_RECURSE PAGES "../pages/*.cpp")

add_library(routes SHARED
    "../libs/Civet/civetweb.c"
    register.cpp
    ${PAGES}
)
set_source_files_properties(register.cpp ${PAGES} PROPERTIES SKIP_AUTOMOC TRUE)
target_link_libraries(routes
 ws2_32
)

target_include_directories(routes
    PRIVATE ${CMAKE_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/libs
    PRIVATE ${CMAKE_SOURCE_DIR}/pages
    PRIVATE ${CMAKE_SOURCE_DIR}/core
)

set_target_properties(routes PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/core"
)

add_definitions(-DNO_SSL)