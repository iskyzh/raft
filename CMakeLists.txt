cmake_minimum_required(VERSION 3.14)
project(raft)

set(CMAKE_CXX_STANDARD 14)

add_library(Raft raft.hpp)
add_executable(RaftTest test_main.cpp)
add_executable(RaftMain main.cpp)

find_package(gRPC CONFIG REQUIRED)

target_link_libraries(Raft PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc_cronet)

find_package(Boost REQUIRED)
target_link_libraries(Raft PRIVATE ${Boost_LIBRARIES})
target_include_directories(Raft PRIVATE ${Boost_INCLUDE_DIRS})

enable_testing()
find_package(GTest MODULE REQUIRED)
target_link_libraries(RaftTest PRIVATE GTest::GTest GTest::Main)
add_test(RaftTests RaftTest)
