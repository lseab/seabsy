cmake_minimum_required(VERSION 4.2.0)

project(seabsy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

# ---- Catch2 via FetchContent ----
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

# ---- Production library ----
add_library(seabsy_lib
  src/arena.hpp
  src/generator.cpp
  src/grammar.hpp
  src/parsing.cpp
  src/scopes.cpp
  src/tokenization.cpp
)
target_include_directories(seabsy_lib PUBLIC src)

# ---- App ----
add_executable(seabsy src/main.cpp)
target_link_libraries(seabsy PRIVATE seabsy_lib)

# ---- Tests ----
add_executable(tests tests/test_main.cpp)
target_link_libraries(tests PRIVATE seabsy_lib Catch2::Catch2WithMain)
