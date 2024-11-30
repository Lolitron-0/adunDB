cmake_minimum_required(VERSION 3.22)
project(adundb)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ADUN_BUILD_TESTS ON)

include_directories(include)

set(SOURCES
    src/Value.cpp
    src/Result.cpp
    src/ResultIterator.cpp
    src/Table.cpp
    src/Column.cpp
    src/Assert.cpp
    src/Database.cpp
    src/Parser/Lexer.cpp
    src/Parser/Token.cpp
    src/Parser/Parser.cpp
    src/Parser/ExpressionNode.cpp
    src/Parser/BinOpExpr.cpp
    src/Parser/UnaryOpExpr.cpp
    src/Parser/CreateCommand.cpp
    src/Parser/InsertCommand.cpp
    src/Parser/SelectCommand.cpp)

add_library(${PROJECT_NAME} ${SOURCES})

# Dependencies
include(FetchContent)

# cul
message(STATUS "Fetching dependency: cul")
FetchContent_Declare(
  cul
  GIT_REPOSITORY https://github.com/Lolitron-0/cul.git
  GIT_TAG c316615e9a5fb3535293082fb187856da01ea492)
FetchContent_MakeAvailable(cul)
target_link_libraries(${PROJECT_NAME} PRIVATE cul)

# frozen
message(STATUS "Fetching dependency: frozen")
FetchContent_Declare(
  frozen
  GIT_REPOSITORY https://github.com/serge-sans-paille/frozen
  GIT_TAG 06bee5321cecd9a9ffaceb164b5adb2ffafae781)
FetchContent_MakeAvailable(frozen)
target_link_libraries(${PROJECT_NAME} PRIVATE frozen)

# fmt
message(STATUS "Fetching dependency: fmt")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG 0c9fce2ffefecfdce794e1859584e25877b7b592) # 11.0.2
FetchContent_MakeAvailable(fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# boost
message(STATUS "Fetching dependency: boost (may take a while)")
add_compile_definitions(BOOST_ENABLE_ASSERT_DEBUG_HANDLER)

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES assert;stacktrace;range)
FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.xz
      DOWNLOAD_EXTRACT_TIMESTAMP
      ON)

FetchContent_MakeAvailable(Boost)

# TODO: pimpl in Value
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::assert)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::range Boost::stacktrace)

# Tests
if(ADUN_BUILD_TESTS)
  add_subdirectory(tests)
endif()
