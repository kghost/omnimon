cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_EXTENSIONS false)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_compile_options("-stdlib=libc++" "-fexperimental-library")

execute_process(
  COMMAND uname -m
  COMMAND sed s/x86_64/x86/
  COMMAND sed s/aarch64/arm64/
  COMMAND sed s/ppc64le/powerpc/
  COMMAND sed s/mips.*/mips/
  COMMAND sed s/riscv64/riscv/
  COMMAND sed s/loongarch.*/loongarch/
  OUTPUT_VARIABLE ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "ARCH: ${ARCH}")

execute_process(
  COMMAND uname -r
  OUTPUT_VARIABLE KERNEL_RELEASE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

find_path(KERNELHEADERS_DIR linux/user.h
          PATHS /lib/modules/${KERNEL_RELEASE}/source/include)

message(STATUS "Kernel release: ${KERNEL_RELEASE}")
message(STATUS "Kernel headers: ${KERNELHEADERS_DIR}")

find_package(PkgConfig)

project(OmniMon)

add_subdirectory(src)
