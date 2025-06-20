# SPDX-License-Identifier: Apache-2.0
# 
# Copyright 2008-2016 Conrad Sanderson (http://conradsanderson.id.au)
# Copyright 2008-2016 National ICT Australia (NICTA)
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------


## NOTE: This installation script generates a runtime wrapper library by default,
## NOTE: which links with OpenBLAS, LAPACK, etc.
## NOTE: 
## NOTE: If you prefer to link your programs directly with OpenBLAS, LAPACK, etc
## NOTE: there are two options:
## NOTE: 
## NOTE: (1) use the HEADER_ONLY option when running this installation script,
## NOTE: or
## NOTE: (2) do not use this installation script; instead compile your programs
## NOTE:     using the include folder directly obtained from the armadillo archive.
## NOTE: 
## NOTE: To forcefully disable use of the runtime wrapper library,
## NOTE: compile your programs with ARMA_DONT_USE_WRAPPER enabled.
## NOTE: Example:
## NOTE: g++ prog.cpp -o prog -O2 -I /home/user/armadillo-14.0.0/include -DARMA_DONT_USE_WRAPPER -lopenblas -llapack
## NOTE: 
## NOTE: The above assumes that the armadillo archive is unpacked into /home/user/  
## NOTE: Adjust for newer versions of Armadillo (change "14.0.0"),
## NOTE: and/or if the armadillo archive has been unpacked into a different folder.
## NOTE: 
## NOTE: Replace -lopenblas with -lblas if you don't have OpenBLAS.
## NOTE: On macOS, replace -lblas -llapack with -framework Accelerate
## NOTE: More details: https://arma.sourceforge.net/faq.html


cmake_minimum_required(VERSION 3.5...3.10 FATAL_ERROR)

if(POLICY CMP0025)
  # enable differentiation between vanilla Clang and AppleClang
  cmake_policy(SET CMP0025 NEW)
  message(STATUS "*** set cmake policy CMP0025 to NEW")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

if(WIN32)
  message(STATUS "")
  message(STATUS "*** WARNING: installation via cmake is not supported on this platform.")
  message(STATUS "*** WARNING: suggest to use manual installation instead;")
  message(STATUS "*** WARNING: see README for details")
  message(STATUS "")
endif()

project(armadillo CXX C)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)


# the settings below will be automatically configured by the rest of this script

set(ARMA_USE_WRAPPER false)
set(ARMA_USE_LAPACK  false)
set(ARMA_USE_BLAS    false)
set(ARMA_USE_ATLAS   false)
set(ARMA_USE_ARPACK  false)
set(ARMA_USE_SUPERLU false)


# extract version from sources

set(ARMA_VERSION_FILE_NAME "${PROJECT_SOURCE_DIR}/include/armadillo_bits/arma_version.hpp")

if(NOT EXISTS ${ARMA_VERSION_FILE_NAME})
  message(FATAL_ERROR "Can't read ${ARMA_VERSION_FILE_NAME}")
endif()

file(READ ${ARMA_VERSION_FILE_NAME} ARMA_VERSION_FILE_CONTENTS)
string(REGEX REPLACE ".*#define ARMA_VERSION_MAJOR ([0-9]+).*" "\\1" ARMA_VERSION_MAJOR "${ARMA_VERSION_FILE_CONTENTS}")
string(REGEX REPLACE ".*#define ARMA_VERSION_MINOR ([0-9]+).*" "\\1" ARMA_VERSION_MINOR "${ARMA_VERSION_FILE_CONTENTS}")
string(REGEX REPLACE ".*#define ARMA_VERSION_PATCH ([0-9]+).*" "\\1" ARMA_VERSION_PATCH "${ARMA_VERSION_FILE_CONTENTS}")

message(STATUS "Configuring Armadillo ${ARMA_VERSION_MAJOR}.${ARMA_VERSION_MINOR}.${ARMA_VERSION_PATCH}")

string(COMPARE EQUAL "${CMAKE_CXX_FLAGS}" "" CXX_FLAGS_EMPTY)
if(NOT CXX_FLAGS_EMPTY)
  message(STATUS "")
  message(STATUS "*** WARNING: variable 'CMAKE_CXX_FLAGS' is not empty; this may cause problems!")
  message(STATUS "")
endif()


if(DEFINED CMAKE_CXX_COMPILER_ID AND DEFINED CMAKE_CXX_COMPILER_VERSION)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(NOT (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 6.1.0))
      if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
        message(STATUS "Added '-std=c++14' to compiler flags")
      endif()
    else()
      message(FATAL_ERROR "Compiler too old; need at least gcc 6.1")
    endif()
  else()
    if(NOT (${CMAKE_MAJOR_VERSION} LESS 3))
      if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 6.0)
          message(STATUS "Detected Clang 6.0 or newer")
          if(NOT DEFINED CMAKE_CXX_STANDARD)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
            message(STATUS "Added '-std=c++14' to compiler flags")
          endif()
        else()
          message(FATAL_ERROR "Compiler too old")
        endif()
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8.0)
          message(STATUS "Detected AppleClang 8.0 or newer")
          if(NOT DEFINED CMAKE_CXX_STANDARD)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
            message(STATUS "Added '-std=c++14' to compiler flags")
          endif()
        else()
          message(FATAL_ERROR "Compiler too old")
        endif()
      endif()
    endif()
  endif()
endif()


# As Red Hat Enterprise Linux (and related systems such as Fedora)
# does not search /usr/local/lib by default, we need to place the
# library in either /usr/lib or /usr/lib64

if(NOT APPLE)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "")
    message(STATUS "*** CMAKE_INSTALL_PREFIX was initialised by cmake to the default value of ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "*** CMAKE_INSTALL_PREFIX changed to /usr")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Standard install prefix" FORCE)
  endif()
endif()


# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

# deprecated options
set(INSTALL_LIB_DIR     "" CACHE STRING "deprecated; use CMAKE_INSTALL_LIBDIR instead")
set(INSTALL_INCLUDE_DIR "" CACHE STRING "deprecated; use CMAKE_INSTALL_INCLUDEDIR instead")
set(INSTALL_DATA_DIR    "" CACHE STRING "deprecated; use CMAKE_INSTALL_DATADIR instead")
set(INSTALL_BIN_DIR     "" CACHE STRING "deprecated; use CMAKE_INSTALL_BINDIR instead")

if(INSTALL_LIB_DIR)
  message(STATUS "*** INSTALL_LIB_DIR is deprecated; use CMAKE_INSTALL_LIBDIR instead")
  set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
  GNUInstallDirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_LIBDIR CMAKE_INSTALL_LIBDIR)
endif()

if(INSTALL_INCLUDE_DIR)
  message(STATUS "*** INSTALL_INCLUDE_DIR is deprecated; use CMAKE_INSTALL_INCLUDEDIR instead")
  set(CMAKE_INSTALL_INCLUDEDIR "${INSTALL_INCLUDE_DIR}")
  GNUInstallDirs_get_absolute_install_dir(CMAKE_INSTALL_FULL_INCLUDEDIR CMAKE_INSTALL_INCLUDEDIR)
endif()

if(INSTALL_DATA_DIR)
  message(STATUS "*** INSTALL_DATA_DIR is deprecated; use CMAKE_INSTALL_DATADIR instead")
  set(CMAKE_INSTALL_DATADIR "${INSTALL_DATA_DIR}")
endif()

if(INSTALL_BIN_DIR)
  message(STATUS "*** INSTALL_BIN_DIR is deprecated; use CMAKE_INSTALL_BINDIR instead")
  set(CMAKE_INSTALL_BINDIR "${INSTALL_BIN_DIR}")
endif()



option(HEADER_ONLY "Do not generate the wrapper library" OFF)
## Example use on the command line:
## cmake -D HEADER_ONLY=true .

if(MSVC)
  option(STATIC_LIB "Generate static library instead of shared library" ON)
else()
  option(STATIC_LIB "Generate static library instead of shared library" OFF)
endif()
## Example use on the command line:
## cmake -D STATIC_LIB=true .

option(OPENBLAS_PROVIDES_LAPACK "Assume that OpenBLAS provides LAPACK functions" OFF)
## Example use on the command line:
## cmake -D OPENBLAS_PROVIDES_LAPACK=true .

option(ALLOW_FLEXIBLAS_LINUX "Allow detection of FlexiBLAS on Linux" ON)
# set(ALLOW_FLEXIBLAS_LINUX false)
## uncomment the above line to disable the detection of FlexiBLAS;
## you can also disable FlexiBLAS detection directly on the command line:
## cmake -D ALLOW_FLEXIBLAS_LINUX=false .

option(ALLOW_OPENBLAS_MACOS "Allow detection of OpenBLAS on macOS" OFF)
## Example use on the command line:
## cmake -D ALLOW_OPENBLAS_MACOS=true .

option(ALLOW_BLAS_LAPACK_MACOS "Allow detection of BLAS and LAPACK on macOS" OFF)
## Example use on the command line:
## cmake -D ALLOW_BLAS_LAPACK_MACOS=true .

option(BUILD_SMOKE_TEST "Build the smoke test" ON)
# set(BUILD_SMOKE_TEST false)
## uncomment the above line to disable building the smoke test;
## you can also disable building the smoke test directly on the command line:
## cmake -D BUILD_SMOKE_TEST=false .
## 
## to run the smoke test on the command line:
## make
## ctest


if(MSVC)
  if(NOT STATIC_LIB)
    message(STATUS "")
    message(STATUS "*** WARNING: building shared library with MSVC is not supported." )
    message(STATUS "*** WARNING: use manual installation instead; see README for details")
    message(STATUS "")
    
    set(STATIC_LIB ON)
  endif()
endif()

if(HEADER_ONLY)
  set(BUILD_WRAPPER false)
else()
  set(BUILD_WRAPPER true)
endif()

if(BUILD_WRAPPER)
  set(ARMA_USE_WRAPPER true)
else()
  set(ARMA_USE_WRAPPER false)
endif()


message(STATUS "")
message(STATUS "CMAKE_SYSTEM_NAME          = ${CMAKE_SYSTEM_NAME}"         )
message(STATUS "CMAKE_CXX_COMPILER_ID      = ${CMAKE_CXX_COMPILER_ID}"     )
message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_COMPILER_IS_GNUCXX   = ${CMAKE_COMPILER_IS_GNUCXX}"  )

message(STATUS "")
message(STATUS "*** options:"                                           )
message(STATUS "HEADER_ONLY               = ${HEADER_ONLY}"             )
message(STATUS "STATIC_LIB                = ${STATIC_LIB}"              )
message(STATUS "OPENBLAS_PROVIDES_LAPACK  = ${OPENBLAS_PROVIDES_LAPACK}")
message(STATUS "ALLOW_FLEXIBLAS_LINUX     = ${ALLOW_FLEXIBLAS_LINUX}"   )
message(STATUS "ALLOW_OPENBLAS_MACOS      = ${ALLOW_OPENBLAS_MACOS}"    )
message(STATUS "ALLOW_BLAS_LAPACK_MACOS   = ${ALLOW_BLAS_LAPACK_MACOS}" )
message(STATUS "BUILD_SMOKE_TEST          = ${BUILD_SMOKE_TEST}"        )

message(STATUS "")
message(STATUS "*** Looking for external libraries")

##
## Find LAPACK and BLAS libraries, or their optimised versions
##

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_aux/Modules/")

if(APPLE)
  message(STATUS "Detected macOS")
  
  set(ARMA_USE_LAPACK     true)
  set(ARMA_USE_BLAS       true)
  set(ARMA_USE_ACCELERATE true)
  
  if(ALLOW_OPENBLAS_MACOS)
    include(ARMA_FindOpenBLAS)
    message(STATUS "OpenBLAS_FOUND = ${OpenBLAS_FOUND}")
    message(STATUS "")
    message(STATUS "*** If use of OpenBLAS is causing problems,")
    message(STATUS "*** rerun cmake with detection of OpenBLAS disabled:")
    message(STATUS "*** cmake -D ALLOW_OPENBLAS_MACOS=false .")
    message(STATUS "")
    
    if(OpenBLAS_FOUND)
      set(ARMA_LIBS ${ARMA_LIBS} ${OpenBLAS_LIBRARIES})
      set(ARMA_USE_ACCELERATE false)
    endif()
  endif()
  
  if(ALLOW_BLAS_LAPACK_MACOS)
    include(ARMA_FindBLAS)
    include(ARMA_FindLAPACK)
    message(STATUS "  BLAS_FOUND = ${BLAS_FOUND}"  )
    message(STATUS "LAPACK_FOUND = ${LAPACK_FOUND}")
    message(STATUS "")
    message(STATUS "*** If use of BLAS and LAPACK is causing problems,")
    message(STATUS "*** rerun cmake with detection of BLAS and LAPACK disabled:")
    message(STATUS "*** cmake -D ALLOW_BLAS_LAPACK_MACOS=false .")
    message(STATUS "")
    
    if(BLAS_FOUND AND LAPACK_FOUND)
      set(ARMA_LIBS ${ARMA_LIBS} ${BLAS_LIBRARIES})
      set(ARMA_LIBS ${ARMA_LIBS} ${LAPACK_LIBRARIES})
      set(ARMA_USE_ACCELERATE false)
    endif()
  endif()
  
  if(ARMA_USE_ACCELERATE)
    set(ARMA_LIBS ${ARMA_LIBS} "-framework Accelerate")  # or "-framework accelerate" ?
    message(STATUS "Added '-framework Accelerate' to compiler flags")
  endif()
  
  if(DEFINED CMAKE_CXX_COMPILER_ID)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
      message(STATUS "Detected Clang compiler on macOS. Added '-stdlib=libc++' to compiler flags")
    endif()
    
    if(NOT (${CMAKE_MAJOR_VERSION} LESS 3))
      if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        message(STATUS "Detected AppleClang compiler on macOS. Added '-stdlib=libc++' to compiler flags")
      endif()
    endif()
  endif()
  
  set(CMAKE_MACOSX_RPATH 1)
  
else()
  
  if(ALLOW_FLEXIBLAS_LINUX AND (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
    include(ARMA_FindFlexiBLAS)
  else()
    set(FlexiBLAS_FOUND false)
  endif()
  
  include(ARMA_FindMKL)
  include(ARMA_FindOpenBLAS)
  include(ARMA_FindATLAS)     # TODO: remove support for ATLAS in next major version
  include(ARMA_FindBLAS)
  include(ARMA_FindLAPACK)
  
  message(STATUS "FlexiBLAS_FOUND = ${FlexiBLAS_FOUND}" )
  message(STATUS "      MKL_FOUND = ${MKL_FOUND}"       )
  message(STATUS " OpenBLAS_FOUND = ${OpenBLAS_FOUND}"  )
  message(STATUS "    ATLAS_FOUND = ${ATLAS_FOUND}"     )
  message(STATUS "     BLAS_FOUND = ${BLAS_FOUND}"      )
  message(STATUS "   LAPACK_FOUND = ${LAPACK_FOUND}"    )
  
  if(FlexiBLAS_FOUND)
    
    set(ARMA_USE_LAPACK true)
    set(ARMA_USE_BLAS   true)
    
    set(ARMA_LIBS ${ARMA_LIBS} ${FlexiBLAS_LIBRARIES})
    
    message(STATUS "")
    message(STATUS "*** Using FlexiBLAS to access BLAS and LAPACK functions.")
    message(STATUS "*** https://www.mpi-magdeburg.mpg.de/projects/flexiblas")
    message(STATUS "*** WARNING: SuperLU and ARPACK must also link with FlexiBLAS.")
    message(STATUS "")
    message(STATUS "*** If using FlexiBLAS causes problems, ")
    message(STATUS "*** rerun cmake with FlexiBLAS detection disabled:")
    message(STATUS "*** cmake -D ALLOW_FLEXIBLAS_LINUX=false .")
    message(STATUS "")
    
  elseif(MKL_FOUND)
    
    set(ARMA_USE_LAPACK true)
    set(ARMA_USE_BLAS   true)
    set(ARMA_LIBS ${ARMA_LIBS} ${MKL_LIBRARIES})
    
    message(STATUS "")
    message(STATUS "*** If the MKL libraries are installed in non-standard locations such as")
    message(STATUS "*** /opt/intel/mkl, /opt/intel/composerxe/, /usr/local/intel/mkl")
    message(STATUS "*** make sure the run-time linker can find them.")
    message(STATUS "*** On Linux systems this can be done by editing /etc/ld.so.conf")
    message(STATUS "*** or modifying the LD_LIBRARY_PATH environment variable.")
    message(STATUS "")
    message(STATUS "*** On systems with SELinux enabled (eg. Fedora, RHEL),")
    message(STATUS "*** you may need to change the SELinux type of all MKL libraries")
    message(STATUS "*** to fix permission problems that may occur during run-time.")
    message(STATUS "")
    
  else()
    
    if(OpenBLAS_FOUND AND ATLAS_FOUND)
      message(STATUS "")
      message(STATUS "*** NOTE: found both OpenBLAS and ATLAS; ATLAS will not be used")
    endif()
    
    if(OpenBLAS_FOUND AND BLAS_FOUND)
      message(STATUS "")
      message(STATUS "*** NOTE: found both OpenBLAS and BLAS; BLAS will not be used")
    endif()
    
    if(OpenBLAS_FOUND)
      
      set(ARMA_USE_BLAS true)
      set(ARMA_LIBS ${ARMA_LIBS} ${OpenBLAS_LIBRARIES})
      
      if(OPENBLAS_PROVIDES_LAPACK)
        set(ARMA_USE_LAPACK true)
      else()
        message(STATUS "")
        message(STATUS "*** NOTE: if OpenBLAS is known to provide LAPACK functions, recommend to")
        message(STATUS "*** NOTE: rerun cmake with the OPENBLAS_PROVIDES_LAPACK option enabled:")
        message(STATUS "*** NOTE: cmake -D OPENBLAS_PROVIDES_LAPACK=true .")
      endif()
      
      message(STATUS "")
      message(STATUS "*** If the OpenBLAS library is installed in")
      message(STATUS "*** /usr/local/lib or /usr/local/lib64")
      message(STATUS "*** make sure the run-time linker can find it.")
      message(STATUS "*** On Linux systems this can be done by editing /etc/ld.so.conf")
      message(STATUS "*** or modifying the LD_LIBRARY_PATH environment variable.")
      message(STATUS "")
      
    else()
      
      if(ATLAS_FOUND)
        set(ARMA_USE_ATLAS true)
        set(ARMA_LIBS ${ARMA_LIBS} ${ATLAS_LIBRARIES})
        
        message(STATUS "")
        message(STATUS "*** NOTE: support for ATLAS is deprecated and will be removed;")
        message(STATUS "*** NOTE: recommend to use OpenBLAS or FlexiBLAS instead.")
        message(STATUS "")
      endif()
      
      if(BLAS_FOUND)
        set(ARMA_USE_BLAS true)
        set(ARMA_LIBS ${ARMA_LIBS} ${BLAS_LIBRARIES})
      endif()
      
    endif()
    
    if(LAPACK_FOUND)
      if(OpenBLAS_FOUND AND OPENBLAS_PROVIDES_LAPACK)
        message(STATUS "*** NOTE: found both OpenBLAS and LAPACK;")
        message(STATUS "*** NOTE: option OPENBLAS_PROVIDES_LAPACK is enabled,")
        message(STATUS "*** NOTE: so will not link with plain LAPACK.")
        message(STATUS "")
      else()
        set(ARMA_USE_LAPACK true)
        set(ARMA_LIBS ${ARMA_LIBS} ${LAPACK_LIBRARIES})
      endif()
    endif()
    
  endif()
  
endif()


include(ARMA_FindARPACK)
message(STATUS "ARPACK_FOUND = ${ARPACK_FOUND}")

if(ARPACK_FOUND)
  set(ARMA_USE_ARPACK true)
  set(ARMA_LIBS ${ARMA_LIBS} ${ARPACK_LIBRARY})
endif()

include(ARMA_FindSuperLU)
message(STATUS "SuperLU_FOUND = ${SuperLU_FOUND}")

if(SuperLU_FOUND)
  message(STATUS "SuperLU_INCLUDE_DIR = ${SuperLU_INCLUDE_DIR}")
  set(ARMA_USE_SUPERLU true)
  set(ARMA_LIBS ${ARMA_LIBS} ${SuperLU_LIBRARY})
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${SuperLU_INCLUDE_DIR})
  set(ARMA_SUPERLU_INCLUDE_DIR ${SuperLU_INCLUDE_DIR})
endif()


if(NOT ARMA_USE_LAPACK)
  message(STATUS "")
  message(STATUS "*** WARNING: Use of LAPACK is not enabled, as no LAPACK compatible library has been found.")
  message(STATUS "*** WARNING: This will materially degrade the available functionality in Armadillo.")

  if(OpenBLAS_FOUND)
    message(STATUS "")
    message(STATUS "*** NOTE: OpenBLAS found but LAPACK not found.")
    message(STATUS "*** NOTE: OpenBLAS may have been built without LAPACK functions,")
    message(STATUS "*** NOTE: so cannot assume that LAPACK functions are available.")
    message(STATUS "*** NOTE: To forcefully assume that OpenBLAS provides LAPACK functions,")
    message(STATUS "*** NOTE: rerun cmake with the OPENBLAS_PROVIDES_LAPACK option enabled:")
    message(STATUS "*** NOTE: cmake -D OPENBLAS_PROVIDES_LAPACK=true .")
  endif()
endif()


message(STATUS "")
message(STATUS "*** Result of configuration:")
message(STATUS "*** ARMA_USE_WRAPPER = ${ARMA_USE_WRAPPER}")
message(STATUS "*** ARMA_USE_LAPACK  = ${ARMA_USE_LAPACK}")
message(STATUS "*** ARMA_USE_BLAS    = ${ARMA_USE_BLAS}")
message(STATUS "*** ARMA_USE_ATLAS   = ${ARMA_USE_ATLAS}")
message(STATUS "*** ARMA_USE_ARPACK  = ${ARMA_USE_ARPACK}")
message(STATUS "*** ARMA_USE_SUPERLU = ${ARMA_USE_SUPERLU}")


if(BUILD_WRAPPER)
  message(STATUS "")
  message(STATUS "*** Armadillo wrapper library links with these libraries:")
  message(STATUS "*** ARMA_LIBS = ${ARMA_LIBS}")
else()
  message(STATUS "")
  message(STATUS "*** Armadillo programs must be manually linked with:")
  message(STATUS "*** ARMA_LIBS = ${ARMA_LIBS}")
endif()


message(STATUS "")
message(STATUS "Copying ${PROJECT_SOURCE_DIR}/include/ to ${PROJECT_BINARY_DIR}/tmp/include/")
file(COPY ${PROJECT_SOURCE_DIR}/include/ DESTINATION ${PROJECT_BINARY_DIR}/tmp/include/)

message(STATUS "Generating ${PROJECT_BINARY_DIR}/tmp/include/armadillo_bits/config.hpp")
configure_file(${PROJECT_BINARY_DIR}/tmp/include/armadillo_bits/config.hpp.cmake ${PROJECT_BINARY_DIR}/tmp/include/armadillo_bits/config.hpp)


include_directories(${PROJECT_BINARY_DIR}/tmp/include/ ${CMAKE_REQUIRED_INCLUDES})


## work around a silly limitation in macOS
if(APPLE)
  if(${ARMA_VERSION_MINOR} GREATER 99)
    math(EXPR ARMA_VERSION_MINOR_ALT "${ARMA_VERSION_MINOR} / 10")
  else()
    set(ARMA_VERSION_MINOR_ALT ${ARMA_VERSION_MINOR})
  endif()
else()
  set(ARMA_VERSION_MINOR_ALT ${ARMA_VERSION_MINOR})
endif()


## necessary when linking with Intel MKL on Linux systems
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()


if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
else()
  if(DEFINED CMAKE_CXX_COMPILER_ID)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    endif()
  endif()
endif()


message(STATUS "")
message(STATUS "CMAKE_CXX_FLAGS           = ${CMAKE_CXX_FLAGS}"          )
message(STATUS "CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "CMAKE_REQUIRED_INCLUDES   = ${CMAKE_REQUIRED_INCLUDES}"  )


if(BUILD_WRAPPER)
  if(STATIC_LIB)
    set(BUILD_SHARED_LIBS OFF)
  else()
    set(BUILD_SHARED_LIBS ON)
  endif()
  
  add_library( armadillo ${PROJECT_SOURCE_DIR}/src/wrapper1.cpp ${PROJECT_SOURCE_DIR}/src/wrapper2.cpp )
  target_link_libraries( armadillo ${ARMA_LIBS} )
else()
  add_library( armadillo INTERFACE )
endif()

# target_include_directories(armadillo INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_include_directories(armadillo INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/tmp/include> $<INSTALL_INTERFACE:include>)
set_target_properties(armadillo PROPERTIES VERSION ${ARMA_VERSION_MAJOR}.${ARMA_VERSION_MINOR_ALT}.${ARMA_VERSION_PATCH} SOVERSION ${ARMA_VERSION_MAJOR})


################################################################################
# INSTALL CONFIGURATION


message(STATUS ""                                                      )
message(STATUS "CMAKE_INSTALL_PREFIX     = ${CMAKE_INSTALL_PREFIX}"    )
message(STATUS "CMAKE_INSTALL_LIBDIR     = ${CMAKE_INSTALL_LIBDIR}"    )
message(STATUS "CMAKE_INSTALL_INCLUDEDIR = ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "CMAKE_INSTALL_DATADIR    = ${CMAKE_INSTALL_DATADIR}"   )
message(STATUS "CMAKE_INSTALL_BINDIR     = ${CMAKE_INSTALL_BINDIR}"    )


# Note that the trailing / character in "include/" is critical

install(DIRECTORY ${PROJECT_BINARY_DIR}/tmp/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
PATTERN ".git" EXCLUDE
PATTERN "*.cmake" EXCLUDE
PATTERN "*~" EXCLUDE
PATTERN "*orig" EXCLUDE
)

install(TARGETS armadillo EXPORT ArmadilloLibraryDepends
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  
## smoke test

if(BUILD_SMOKE_TEST)
  if(HEADER_ONLY)
    ## TODO: update to link smoke_test with ARMA_LIBS when HEADER_ONLY option is enabled
    message(STATUS "*** building smoke_test not supported when HEADER_ONLY enabled")
  else()
    message(STATUS "*** configuring smoke_test")
    enable_testing()
    add_subdirectory(tests1)
  endif()
endif()

## pkgconfig related
## TODO: this currently doesn't handle the required SuperLU include dir
## TODO: update armadillo.pc.in to link with ARMA_LIBS when HEADER_ONLY option is enabled

if(HEADER_ONLY)
  message(STATUS "*** generating pkgconfig files not supported when HEADER_ONLY enabled")
else()
  message(STATUS "*** copying ${PROJECT_SOURCE_DIR}/misc/ to ${PROJECT_BINARY_DIR}/tmp/misc/")
  file(COPY ${PROJECT_SOURCE_DIR}/misc/ DESTINATION ${PROJECT_BINARY_DIR}/tmp/misc/)
  
  message(STATUS "*** generating '${PROJECT_BINARY_DIR}/tmp/misc/armadillo.pc'")
  configure_file(${PROJECT_BINARY_DIR}/tmp/misc/armadillo.pc.in "${PROJECT_BINARY_DIR}/tmp/misc/armadillo.pc" @ONLY)
  install(FILES "${PROJECT_BINARY_DIR}/tmp/misc/armadillo.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()


if(HEADER_ONLY)
  message(STATUS "")
  message(STATUS "*** HEADER_ONLY mode is enabled; skipping rest of configuration script")
  return()
endif()


# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE armadillo)

## LOCAL FILES
# Create ArmadilloConfig.cmake file for the use from the build tree
set(ARMADILLO_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/tmp/include")
set(ARMADILLO_LIB_DIR      "${PROJECT_BINARY_DIR}")
set(ARMADILLO_CMAKE_DIR    "${PROJECT_BINARY_DIR}")

message(STATUS "Generating '${PROJECT_BINARY_DIR}/ArmadilloConfig.cmake'")
# copy/change config and configVersion file (modify only the @xyz@ variables)
configure_file(${PROJECT_SOURCE_DIR}/cmake_aux/InstallFiles/ArmadilloConfig.cmake.in
  "${PROJECT_BINARY_DIR}/ArmadilloConfig.cmake" @ONLY)

message(STATUS "Generating '${PROJECT_BINARY_DIR}/ArmadilloConfigVersion.cmake'")
configure_file(${PROJECT_SOURCE_DIR}/cmake_aux/InstallFiles/ArmadilloConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/ArmadilloConfigVersion.cmake" @ONLY)

# Install the export set for use with the install-tree
install(EXPORT ArmadilloLibraryDepends DESTINATION
  "${CMAKE_INSTALL_DATADIR}/Armadillo/CMake"
  COMPONENT dev)


## GLOBAL INSTALL FILES
# Create ArmadilloConfig.cmake file for the use from the install tree
# and install it
set(ARMADILLO_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(ARMADILLO_LIB_DIR      "${CMAKE_INSTALL_FULL_LIBDIR}")
set(ARMADILLO_CMAKE_DIR    "${CMAKE_INSTALL_FULL_DATADIR}/Armadillo/CMake")


message(STATUS "Generating '${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfig.cmake'")
# copy/change config and configVersion file (modify only the @xyz@ variables)
configure_file(${PROJECT_SOURCE_DIR}/cmake_aux/InstallFiles/ArmadilloConfig.cmake.in
  "${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfig.cmake" @ONLY)

message(STATUS "Generating '${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfigVersion.cmake'")
configure_file(${PROJECT_SOURCE_DIR}/cmake_aux/InstallFiles/ArmadilloConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfigVersion.cmake" @ONLY)

# Install files to be found by cmake users with find_package()
install(FILES
  "${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfig.cmake"
  "${PROJECT_BINARY_DIR}/InstallFiles/ArmadilloConfigVersion.cmake"
  DESTINATION "${ARMADILLO_CMAKE_DIR}" COMPONENT dev)
