cmake_minimum_required(VERSION 3.10)

# 交叉工具链：由环境变量显式指定，禁止自动探测
if(DEFINED ENV{CROSS_C_COMPILER} AND DEFINED ENV{CROSS_CXX_COMPILER})
  set(CMAKE_C_COMPILER "$ENV{CROSS_C_COMPILER}")
  set(CMAKE_CXX_COMPILER "$ENV{CROSS_CXX_COMPILER}")
elseif(DEFINED ENV{CROSS_TOOLCHAIN_PREFIX})
  set(CMAKE_C_COMPILER "$ENV{CROSS_TOOLCHAIN_PREFIX}gcc")
  set(CMAKE_CXX_COMPILER "$ENV{CROSS_TOOLCHAIN_PREFIX}g++")
else()
  message(FATAL_ERROR "Set CROSS_TOOLCHAIN_PREFIX or CROSS_{C,CXX}_COMPILER from the target Youdao device profile")
endif()

project(jsapi_tsctl C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SDK "${CMAKE_SOURCE_DIR}/iot-miniapp-sdk")

file(GLOB_RECURSE SDK_SOURCES "${SDK}/src/*.cpp")
add_library(iot_sdk STATIC ${SDK_SOURCES})
target_compile_options(iot_sdk PRIVATE -w)
target_include_directories(iot_sdk PUBLIC "${SDK}/include")
set_target_properties(iot_sdk PROPERTIES POSITION_INDEPENDENT_CODE ON)

file(GLOB_RECURSE PLUGIN_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_library(jsapi_tsctl SHARED ${PLUGIN_SOURCES})
target_include_directories(jsapi_tsctl PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_options(jsapi_tsctl PRIVATE
  -Wall -Wextra -Werror=return-type -Wno-psabi -fvisibility=hidden)
target_link_libraries(jsapi_tsctl PRIVATE
  iot_sdk pthread -Wl,-unresolved-symbols=ignore-all)
set_target_properties(jsapi_tsctl PROPERTIES
  OUTPUT_NAME "jsapi_tsctl"
  PREFIX "lib")
