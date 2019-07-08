cmake_minimum_required(VERSION 3.12)
project(raft)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf CONFIG REQUIRED)
find_package(GRPC CONFIG REQUIRED)
find_package(Boost REQUIRED)

include_directories(protos ${Boost_INCLUDE_DIRS})

add_library(protos protos/raft.pb.cc protos/raft.grpc.pb.cc)
add_library(raft src/raft.cpp src/raft.h src/Instance.cpp src/Instance.h src/LogStorage.cpp src/LogStorage.h src/MockRPCService.cpp src/MockRPCService.h src/MockRPCClient.cpp src/MockRPCClient.h src/MockRPC.h src/utils.h)
target_link_libraries(raft PRIVATE protos ${Boost_LIBRARIES} protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc_cronet)
target_include_directories(raft PRIVATE)

add_executable(RaftMain src/main.cpp src/utils.cpp)
target_link_libraries(RaftMain raft)

add_executable(RaftTest test_main.cpp src/LogStorage_test.cpp src/Instance_test.cpp src/MockRPC_test.cpp src/utils_mock.cpp)
enable_testing()
find_package(GTest MODULE REQUIRED)
target_link_libraries(RaftTest PRIVATE raft GTest::GTest GTest::Main)
add_test(RaftTests RaftTest)
