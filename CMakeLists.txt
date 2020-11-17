cmake_minimum_required(VERSION 3.17)
project(micro-clang)

set(CMAKE_CXX_STANDARD 14)

find_package(BISON)
find_package(FLEX)
FLEX_TARGET(scanner scanner.l ${PROJECT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(parser parser.y ${PROJECT_BINARY_DIR}/parser.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
            )

ADD_FLEX_BISON_DEPENDENCY(scanner parser)

add_library(micro_c_parser ${BISON_parser_OUTPUTS}
              ${FLEX_scanner_OUTPUTS}
              )
              
add_executable(micro-clang main.cpp)
target_link_libraries(micro-clang micro_c_parser)
find_library(LEX_LIB l)
target_link_libraries(micro-clang ${LEX_LIB})