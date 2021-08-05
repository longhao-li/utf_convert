cmake_minimum_required(VERSION 3.5)

project(utf_convert CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(CMAKE_CXX_STANDARD 98)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC)
aux_source_directory(test TEST_SRC)

add_library(utf_convert STATIC ${SRC})
add_executable(test_u8_to_u32 ${TEST_SRC})

target_link_libraries(test_u8_to_u32 utf_convert)

add_test(
    NAME test1 
    COMMAND test_u8_to_u32 data/utf8_1.txt data/utf32_1.txt 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test
)
