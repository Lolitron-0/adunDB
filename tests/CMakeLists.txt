set(PARENT_PROJECT_NAME ${PROJECT_NAME})
project(${PARENT_PROJECT_NAME}_tests)

include(FetchContent)

# Unit tests
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
      DOWNLOAD_EXTRACT_TIMESTAMP
      ON)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(UNIT_PROJECT_NAME ${PROJECT_NAME}_unit)
add_executable(${UNIT_PROJECT_NAME} gtest.cpp)
target_link_libraries(${UNIT_PROJECT_NAME} PRIVATE ${PARENT_PROJECT_NAME})
target_link_libraries(${UNIT_PROJECT_NAME} PRIVATE GTest::gtest_main
                                                   GTest::gtest)

include(GoogleTest)
gtest_discover_tests(${UNIT_PROJECT_NAME})

# Sandbox
set(SANDBOX_PROJECT_NAME ${PROJECT_NAME}_sandbox)
add_executable(${SANDBOX_PROJECT_NAME} Sandbox.cpp)
target_link_libraries(${SANDBOX_PROJECT_NAME} PRIVATE ${PARENT_PROJECT_NAME})
