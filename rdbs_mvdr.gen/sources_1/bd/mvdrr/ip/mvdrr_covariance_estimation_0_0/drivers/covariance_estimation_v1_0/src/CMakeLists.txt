# ==============================================================
# Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
# Tool Version Limit: 2025.11
# Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
# 
# ==============================================================
cmake_minimum_required(VERSION 3.15)
project(covariance_estimation)
find_package(common)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_SOURCES xcovariance_estimation.c)
collect (PROJECT_LIB_SOURCES xcovariance_estimation_sinit.c)
collect (PROJECT_LIB_SOURCES xcovariance_estimation_linux.c)
collect (PROJECT_LIB_SOURCES xcovariance_estimation_g.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collect (PROJECT_LIB_HEADERS xcovariance_estimation.h)
collect (PROJECT_LIB_HEADERS xcovariance_estimation_hw.h)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)
add_library(covariance_estimation STATIC ${_sources})
set_target_properties(covariance_estimation PROPERTIES LINKER_LANGUAGE C)

