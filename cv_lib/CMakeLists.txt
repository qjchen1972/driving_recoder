set(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake")
set(ANDROID_ABI armeabi-v7a)
set(ANDROID_ARM_NEON  ON)
set(ANDROID_PLATFORM android-14)
#set(ANDROID_NO_UNDEFINED OFF)
set(ANDROID_SO_UNDEFINED ON)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

cmake_minimum_required(VERSION 2.8.10)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release CACHE STRING "Choose the type of build" FORCE)
endif()

project(tool)

find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Release|RELEASE|release)")
    add_definitions(-Ofast)
    add_definitions(-ffast-math)
endif()

if(PI3)
    add_definitions(-march=native)
    add_definitions(-mfpu=neon)
    add_definitions(-mfloat-abi=hard)
    add_definitions(-D__ARM_NEON)
    add_definitions(-D__ANDROID__)
endif()

#add_definitions(-Wall -Wextra -Wno-unused-function)
add_definitions(-w)
add_definitions(-fPIC)
add_definitions(-std=c++11)
#add_definitions(-fuse-ld=bfd)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(mtcnn)
add_subdirectory(face_lib)

