cmake_minimum_required(VERSION 3.26)
project(cppsqlx VERSION 0.1.0 LANGUAGES CXX)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_EXAMPLES "Build examples" ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)

install(TARGETS cppsqlx
    EXPORT cppsqlxTargets
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/cppsqlx DESTINATION include)

export(EXPORT cppsqlxTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cppsqlxTargets.cmake"
    NAMESPACE cppsqlx::
)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

