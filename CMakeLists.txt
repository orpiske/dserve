project(dserve)
cmake_minimum_required(VERSION 2.8)

include_directories(src/common src/common/network)

add_subdirectory(src/client build/client)

include_directories(src/server/downstream src/server/upstream)

add_subdirectory(src/server build/server)