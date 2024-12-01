set(PARENT_PROJECT_NAME ${PROJECT_NAME})
project(${PARENT_PROJECT_NAME}_fuzz)

set(FUZZ_PROJECT_NAME ${PROJECT_NAME}_parser)
add_executable(${FUZZ_PROJECT_NAME} fuzz_parser.cpp)
target_link_libraries(${FUZZ_PROJECT_NAME} PRIVATE ${PARENT_PROJECT_NAME} cul)
