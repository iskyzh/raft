cmake_minimum_required(VERSION 3.7)
project(raft)

set(CMAKE_CXX_STANDARD 14)

set(Boost_USE_STATIC_LIBS ON)

find_package(protobuf CONFIG REQUIRED)
find_package(GTest MODULE REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Boost COMPONENTS log REQUIRED)

include_directories(protos ${GTEST_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(protos protos/raft.pb.cc protos/raft.grpc.pb.cc)
add_library(core core/Instance.cpp core/Instance.h core/LogStorage.cpp core/LogStorage.h core/MockRPCService.cpp core/MockRPCService.h core/MockRPCClient.cpp core/MockRPCClient.h core/RPC.h utils/utils.h core/RPCClient.h)
target_link_libraries(core protos ${Boost_LIBRARIES}
        protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
        gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc_cronet)

add_executable(RaftMockMain src/mock_main.cpp utils/utils.cpp)
target_link_libraries(RaftMockMain core)

add_executable(RaftTest core/LogStorage_test.cpp core/Instance_test.cpp core/MockRPC_test.cpp utils/utils_mock.cpp src/test_main.cpp)
target_link_libraries(RaftTest PRIVATE core GTest::GTest GTest::Main)

add_executable(RaftMockRPC src/grpc_main.cpp utils/utils.cpp rpc/grpc_client.hpp)
target_link_libraries(RaftMockRPC core)

add_executable(RaftMain src/service_main.cpp utils/utils.cpp)
target_link_libraries(RaftMain core)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
