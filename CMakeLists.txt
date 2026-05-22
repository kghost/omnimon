cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set(CMAKE_C_COMPILER "/usr/bin/clang-16")
# set(CMAKE_CXX_COMPILER "/usr/bin/clang++-16")
set(CMAKE_CXX_EXTENSIONS true)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

set(FTXUI_BUILD_EXAMPLES OFF)
set(FTXUI_ENABLE_INSTALL OFF)

# add_compile_definitions(_LIBCPP_ENABLE_EXPERIMENTAL)
# add_compile_options("-stdlib=libc++" "-fexperimental-library")
# add_link_options("-stdlib=libc++" "-lc++experimental")

add_compile_options("-fno-omit-frame-pointer")

option(OMNIMON_ENABLE_ASAN "Enable AddressSanitizer for host targets" ON)

if(OMNIMON_ENABLE_ASAN)
  add_compile_options("-fsanitize=address")
  add_link_options("-fsanitize=address")
endif()

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

project(OmniMon)

enable_testing()

find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(external/ftxui)
add_subdirectory(src)

