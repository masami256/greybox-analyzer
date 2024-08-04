cmake_minimum_required(VERSION 3.6)
project(GreyboxAnalyzer)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Using LLVM version ${LLVM_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_definitions(${LLVM_DEFINITIONS})
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)

find_package(Boost REQUIRED COMPONENTS system filesystem log)
include_directories(${Boost_INCLUDE_DIRS})

set(LLVM_LINK_COMPONENTS
    Analysis
    Support)

add_llvm_executable(analyzer
  main.cpp)

target_link_libraries(analyzer PRIVATE ${Boost_LIBRARIES} ${LLVM_LIBRARIES})