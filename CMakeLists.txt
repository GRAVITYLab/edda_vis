# Copyright 2015 The Edda Authors. All rights reserved.
# Use of this source code is governed by a MIT-style license that can be
# found in the LICENSE file.

cmake_minimum_required (VERSION 2.8)

# Set the project's name.
project (edda_vis)

### Provide options that the user can optionally select.
option (EDDA_BUILD_VTK_EXAMPLES "Build the example programs." ON)
option (EDDA_BUILD_SHARED_LIBS "Build into shared libraries." ON)
option (EDDA_BUILD_PARAVIEW_PLUGINS "Build Paraview Plugins." OFF)
set (EDDA_INSTALL_DIR "/usr/local" CACHE PATH "EDDA build path")

set (THRUST_DEVICE_SYSTEM "CUDA" CACHE STRING "Thrust device backend (CUDA/TBB/OPENMP/CPP).")
set_property(CACHE THRUST_DEVICE_SYSTEM PROPERTY STRINGS "CUDA;TBB;OPENMP;CPP")
###

# Tell the compiler to use C++11.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(WARNING "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


# All warnings on
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall)


# Load settings for the boost project.
find_package (Boost REQUIRED)
if (Boost_FOUND)
  include_directories (${Boost_INCLUDE_DIRS})
  link_libraries (${Boost_LIBRARIES})
endif ()

# default CUDA off
set(EDDA_BUILD_WITH_CUDA OFF)

# To fix compilation problem: relocation R_X86_64_32 against `a local symbol' can not be
# used when making a shared object; recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
ENDIF()

## Thrust with CUDA backend
if (${THRUST_DEVICE_SYSTEM} STREQUAL "CUDA")
  find_package(CUDA REQUIRED)

  #list(APPEND CUDA_NVCC_FLAGS "-ffast-math")
  #list(APPEND CUDA_NVCC_FLAGS --compiler-options -fno-strict-aliasing -lineinfo -use_fast_math -Xptxas -dlcm=cg)
  #list(APPEND CUDA_NVCC_FLAGS  "--std=c++11") #cuda 7 feature
  #list(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_20,code=sm_20)
  #list(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_30,code=sm_30)
  #list(APPEND CUDA_NVCC_FLAGS -gencode arch=compute_35,code=sm_35)

  # To successfully build a library, this is required (rdc=true)
  # See http://stackoverflow.com/questions/13683575/cuda-5-0-separate-compilation-of-library-with-cmake
  set(CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE ON)
  set(CUDA_SEPARABLE_COMPILATION ON)

  list(APPEND LINK_LIBS ${CUDA_LIBRARIES} ${CUDA_CUDART_LIBRARY})

  set(EDDA_BUILD_WITH_CUDA ON)

## Thrust with TBB backend
elseif (${THRUST_DEVICE_SYSTEM} STREQUAL "TBB")

  add_definitions(-DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_TBB)

  set ( THRUST_INCLUDE_DIR "/usr/local/include" CACHE PATH "Thrust root dir")
  include_directories ( ${THRUST_INCLUDE_DIR} )

  set ( TBB_INSTALL_PATH "/usr/local" CACHE PATH "TBB root dir")
  include_directories(${TBB_INSTALL_PATH}/include)
  link_directories( ${TBB_INSTALL_PATH}/lib)
  list(APPEND LINK_LIBS tbb)

## Thrust with OpenMP backend
elseif (${THRUST_DEVICE_SYSTEM} STREQUAL "OPENMP")
  add_definitions ( -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP)

  set ( THRUST_INCLUDE_DIR "/usr/local/include" CACHE PATH "Thrust root dir")
  include_directories ( ${THRUST_INCLUDE_DIR} )

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

elseif (${THRUST_DEVICE_SYSTEM} STREQUAL "CPP")
  add_definitions ( -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_CPP)

  set ( THRUST_INCLUDE_DIR "/usr/local/include" CACHE PATH "Thrust root dir")
  include_directories ( ${THRUST_INCLUDE_DIR} )

else ()
  message(ERROR "Invalid setting of THRUST_DEVICE_SYSTEM")
endif ()


### Set libraries' output path to lib.
##set (LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# Set variables for platform detection.
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (OS_LINUX ON)
  add_definitions(-DOS_LINUX)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set (OS_MACOSX ON)
  add_definitions(-DOS_MACOSX)
endif ()

if (WIN32)
  set (OS_WIN ON)
  add_definitions(-DOS_WIN)
endif ()

if (${EDDA_BUILD_SHARED_LIBS})
    set(LIB_TYPE SHARED)
else ()
    set(LIB_TYPE STATIC)
endif ()


# Choose either Paraview or VTK library
if (EDDA_BUILD_PARAVIEW_PLUGINS)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
  list(APPEND LINK_LIBS ${VTK_LIBRARIES})

else ()
  # VTK integration
  find_package(VTK)
  include (${VTK_USE_FILE})
  list(APPEND LINK_LIBS ${VTK_LIBRARIES})

endif ()


#################### VTK fix for Cuda #######################
macro(REMOVE_VTK_DEFINITIONS)
    get_directory_property(_dir_defs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
    set(_vtk_definitions)
    foreach(_item ${_dir_defs})
        if(_item MATCHES "vtk*")
            list(APPEND _vtk_definitions -D${_item})
        endif()
    endforeach()
    remove_definitions(${_vtk_definitions})
endmacro(REMOVE_VTK_DEFINITIONS)

REMOVE_VTK_DEFINITIONS()



# Add include directories to the build.
include_directories (${EDDA_INSTALL_DIR}/include/edda)
include_directories (${CMAKE_SOURCE_DIR}/filters)
include_directories (${CMAKE_SOURCE_DIR}/src)
link_directories( ${EDDA_INSTALL_DIR}/lib/edda )

# Add the src subdirectory to the build.
add_subdirectory (src)

# If EDDA_BUILD_VTK_EXAMPLES is on, add the examples subdirectory to the build.
if (EDDA_BUILD_VTK_EXAMPLES)
  add_subdirectory (vtk-examples)
endif ()

