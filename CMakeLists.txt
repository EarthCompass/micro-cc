cmake_minimum_required(VERSION 3.17)
project(micro-cc)

set(CMAKE_CXX_STANDARD 14)

# disable warning
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")

set(LLVM_DIR clang+llvm-11.0.0-x86_64-apple-darwin/lib/cmake/llvm)


find_package(LLVM)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Found LLVM includes in: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(${PROJECT_SOURCE_DIR})

find_package(BISON)
find_package(FLEX)
FLEX_TARGET(scanner scanner.l ${PROJECT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(parser parser.y ${PROJECT_BINARY_DIR}/parser.cpp 
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h
            )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

add_library(micro_c_parser ${BISON_parser_OUTPUTS}
              ${FLEX_scanner_OUTPUTS}
              )
              
add_executable(micro-cc main.cpp codegen.h)
target_link_libraries(micro-cc micro_c_parser)
# find_library(LEX_LIB l)

llvm_map_components_to_libnames(llvm_libs support core irreader native)
#execute_process(COMMAND ${LLVM_INCLUDE_DIRS}/../bin/llvm-config --libs all
#        RESULT_VARIABLE llvm_libs)
message(STATUS "LLVM libs: ${llvm_libs}")
target_link_libraries(micro-cc ${LEX_LIB} ${llvm_libs})