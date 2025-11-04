
# Find all .cpp files in core/handler/ (route_register.cpp)
file(GLOB_RECURSE HANDLER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Find all .cpp files in main/routes/ (user route files like test.cpp)
file(GLOB_RECURSE ROUTE_FILES "${CMAKE_SOURCE_DIR}/main/routes/*.cpp")

# Combine both
set(ROUTES ${HANDLER_FILES} ${ROUTE_FILES})

# Debug: Print what files were found
message(STATUS "Handler files: ${HANDLER_FILES}")
message(STATUS "Route files: ${ROUTE_FILES}")
message(STATUS "All library sources: ${ROUTES}")

# Create a shared library from the found .cpp files
add_library(routes SHARED ${ROUTES})

# Set the output directory for the library
set_target_properties(routes PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_NAME "routes"
)

# Specify the include directories for the target `routes`
target_include_directories(routes 
    PUBLIC ${CMAKE_SOURCE_DIR}/core/handler/
    PUBLIC ${CMAKE_SOURCE_DIR}/core/libs/
    PUBLIC ${CMAKE_SOURCE_DIR}/core/phoenix/
    PUBLIC ${CMAKE_SOURCE_DIR}/main/routes/
)

# Link against the 'dl' library (dynamic linking)
# Note: We don't link against CivetWeb here because the symbols
# will be resolved from the main Phoenix executable at runtime
target_link_libraries(routes PRIVATE dl)

# Allow undefined symbols (they'll be resolved by the main executable)
if(NOT WIN32)
    target_link_options(routes PRIVATE "-Wl,--allow-shlib-undefined")
endif()

