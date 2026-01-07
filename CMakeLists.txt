cmake_minimum_required(VERSION 3.20)
project(VenomEngine LANGUAGES CXX)

# Force C++23 Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Determine Compiler-Specific Optimization Flags
if(MSVC)
    # Windows / Visual Studio
    add_compile_options(/W4 /Ot /Oi /arch:AVX512)
    add_definitions(-DNOMINMAX)
else()
    # Linux / macOS (GCC or Clang)
    # -mavx512f: Foundation instructions
    # -mavx512dq: Doubleword/Quadword instructions
    # -O3: Maximum optimization
    # -march=native: Tune for the current machine's CPU
    add_compile_options(-O3 -mavx512f -mavx512dq -march=native -Wall -Wextra -pthread)
endif()

# Source Files
set(SOURCES 
    src/main.cpp 
    src/OrderBook.cpp 
    src/RiskEngine.cpp
)

# Include Directories
include_directories(include)

# Create Executable
add_executable(VenomEngine ${SOURCES})

# Explicitly link threads for Linux
find_package(Threads REQUIRED)
target_link_libraries(VenomEngine PRIVATE Threads::Threads)

# Print Optimization Status
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")