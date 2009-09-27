# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


if(DOCUMENTATION_ONLY)
  # nothing here
elseif(NOT ALL_IN_ONE)
  # cmdline
  # BOOST configuraton:
  # 1. boost without install, maybe it is needed to copy libs to lib/
  #    env BOOST_ROOT=/home/jarda/src/boost_1_38_0 ...
  #
  # 2. boost install
  #    $ ./configure --prefix=/usr/local --with-libraries=filesystem,system
  #    $ make install
  #    $ ln -s /usr/local/include/boost-1_38/ /usr/local/include/boost/
  #    then cmake will find boost in /usr/local
  #
  # cmake \
  #     -DJAG_THIRD_PARTY_FROM_SYSTEM=on
  #     -DCMAKE_LIBRARY_PATH=/usr/local/lib
  #     -DCMAKE_INCLUDE_PATH=/usr/local/include
  #     ../jagbase
  #
  # TODO
  # - check versions
  #
  # boost
  #set(Boost_DEBUG on)


  FIND_PACKAGE(Boost ${REQ_BOOST_VERSION})
  if (Boost_FOUND)
    #message(STATUS ">> ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
      list(APPEND JAG_THIRD_PARTY_LIBS ${Boost_LIBRARIES})
  else()
      message(SEND_ERROR "Boost C++ libraries not found")
  endif()
  # free type
  FIND_PACKAGE(Freetype ${REQ_FREETYPE_VERSION} REQUIRED)
  check_freetype_version(${REQ_FREETYPE_VERSION} "${FREETYPE_INCLUDE_DIRS}")
  include_directories(${FREETYPE_INCLUDE_DIRS})
  list(APPEND JAG_THIRD_PARTY_LIBS ${FREETYPE_LIBRARIES})
  # zlib
  FIND_PACKAGE(ZLIB ${REQ_ZLIB_VERSION} REQUIRED)
  check_zlib_version(${REQ_ZLIB_VERSION} "${ZLIB_INCLUDE_DIR}")
  include_directories(${ZLIB_INCLUDE_DIR})
  list(APPEND JAG_THIRD_PARTY_LIBS ${ZLIB_LIBRARIES})
  # png
  FIND_PACKAGE(PNG ${REQ_PNG_VERSION} REQUIRED)
  check_png_version(${REQ_PNG_VERSION} "${PNG_INCLUDE_DIR}")
  include_directories(${PNG_INCLUDE_DIR})
  list(APPEND JAG_THIRD_PARTY_LIBS ${PNG_LIBRARIES})
  # icu
  FIND_PACKAGE(ICU ${REQ_ICU_VERSION} REQUIRED)
  check_icu_version(${REQ_ICU_VERSION} "${ICU_INCLUDE_DIRS}")
  include_directories(${ICU_INCLUDE_DIRS})
  list(APPEND JAG_THIRD_PARTY_LIBS ${ICU_LIBRARIES})
  #
  list(REMOVE_DUPLICATES JAG_THIRD_PARTY_LIBS)
#   message(STATUS "LIBS: ${JAG_THIRD_PARTY_LIBS}")
#   get_directory_property(THIRD_INCLUDES INCLUDE_DIRECTORIES)
#   message(STATUS "INCL: ${THIRD_INCLUDES}")
else()
  FIND_PACKAGE(Boost 1.36)
  # use our files
  include_directories(BEFORE ${CMAKE_SOURCE_DIR}/external/include/zlib)
  include_directories(BEFORE ${CMAKE_SOURCE_DIR}/external/include/freetype)
  include_directories(BEFORE ${CMAKE_SOURCE_DIR}/external/include/png)
  include_directories(BEFORE ${CMAKE_SOURCE_DIR}/external/include/icu)
  # ICU is linked statically, so define the following:
  add_definitions(-DU_STATIC_IMPLEMENTATION)
  if (JAG_GCC_IN_UNIX)
    set(JAG_THIRD_PARTY_LIBDIR "${CMAKE_SOURCE_DIR}/external/lib/linux-x86-32-gcc-${JAG_GCC_VERSION_STRING}")
    #
    # Allows linking libstdc++ statically. For more details on the subject see
    # http://www.trilithium.com/johan/2005/06/static-libstdc/
    #
    # In short, the trick is to create a link to libstdc++.a in a special directory
    # and pass that directory to gcc via -L along with '-static-libgcc'. In our case
    # we have a copy of libstdc++.a in our codebase
    #
    set(LIBSTDCPP_LINK_FLAG "-static-libgcc")
  elseif(MSVC_VERSION)
    set(JAG_THIRD_PARTY_LIBDIR "${CMAKE_SOURCE_DIR}/external/lib/windows-x86-32-msvc-${MSVC_MAJOR}.${MSVC_MINOR}")
  endif()
  if (NOT IS_DIRECTORY ${JAG_THIRD_PARTY_LIBDIR})
    message(FATAL_ERROR "Third party libraries are not available for this compiler.")
  endif()
  link_directories(${JAG_THIRD_PARTY_LIBDIR}/static)
  link_directories(${JAG_THIRD_PARTY_LIBDIR})
  # forms JAG_THIRD_PARTY_LIBS which is a list of 3rd party libraries we link with
  if(JAG_GCC_IN_UNIX)
    set(JAG_THIRD_PARTY_LIBS
      z
      freetype
      png
      sicuuc
      sicudata)
  elseif(MSVC_VERSION)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(JAG_THIRD_PARTY_LIBS
        zlibd
        freetype235MT_D
        libpngd
        sicuucd
        sicudtd)
    else()
      set(JAG_THIRD_PARTY_LIBS
        zlibr
        freetype235MT
        libpng
        sicuuc
        sicudt)
    endif()
  endif()
  # in Windows/MSVC link statically with runtime
  if(MSVC_VERSION)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _JAG_BUILD_TYPE)
    foreach(lang "C" "CXX")
      string(REGEX REPLACE "/MDd|/MTd|/MD|/MT" "" "CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE}" "${CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE}}")
      if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE} "/MTd ${CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE}}")
      else()
        set(CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE} "/MT ${CMAKE_${lang}_FLAGS_${_JAG_BUILD_TYPE}}")
      endif()
    endforeach(lang)
  endif()
endif()


# ---------------------------------------------------------------------------
#                           global includes
#
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/code/include)
include_directories(${CMAKE_SOURCE_DIR}/code/include/external/boost-unofficial)
set(MSG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include/messages)
include_directories(${MSG_INCLUDE_DIR})



# ---------------------------------------------------------------------------
#                         compiler/linker settings
#
if(CMAKE_BUILD_TOOL STREQUAL nmake)
  # Do not use precompiled headers with nmake makefiles. Otherwise nmake builds
  # *all* targets because it can't find precompiled.cpp.obj even though it is up
  # to date.
  set(JAG_PRECOMPILED_HEADERS OFF CACHE BOOL "Whether to use precompiled headers.")
else()
  set(JAG_PRECOMPILED_HEADERS ON CACHE BOOL "Whether to use precompiled headers.")
endif()

#
# GCC
#
if(JAG_GCC)
  # debug only flags
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-inline")
  endif()
  # Linux specific, but might be applicable to other platforms as well
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -pthread -D_REENTRANT") # taken from boost - for Linux only
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC -fvisibility=hidden -ftemplate-depth-128 -fdiagnostics-show-option")
    # -Werror disabled as it seems that regardless of using -Wunused-parameter an error is triggered
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall -Wsign-compare -Wunused-parameter -Wunused-variable")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -frtti")
    set(JAG_LINK_LIBS pthread rt)
  endif()
  # version specific
  if (JAG_GCC_VERSION_STRING VERSION_LESS 4.2.0)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-non-virtual-dtor")
  endif()
#
# MSVC
#
elseif(MSVC_VERSION)
  if (ALL_IN_ONE)
    # treat warning as errors only for all-in-one mode
    set(COMPILE_FLAGS "${COMPILE_FLAGS} /WX") # treat warnings as errors
  endif()
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /W4") # warning level 4
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4512") # l4 'class' : assignment operator could not be generated
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4127") # l4 conditional expression is constant
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4510") # l4 default constructor could not be generated
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4610") # l4 object 'class' can never be instantiated - user-defined constructor required
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4800") # l3 'type' : forcing value to bool 'true' or 'false' (performance warning)
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4996") # l1 'function': was declared deprecated
  set(COMPILE_FLAGS "${COMPILE_FLAGS} /wd4706") # l4 assignment within conditional expression
  string(REGEX REPLACE "/W[01234]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

# set the flags to cmake variables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}")


# ---------------------------------------------------------------------------
#                           variables
#
set(API_SWIG_DIR ${CMAKE_BINARY_DIR}/swig)
set(API_HEADER_DIR ${CMAKE_BINARY_DIR}/include/jagpdf/detail)


# ---------------------------------------------------------------------------
#                           subprojects
#
add_subdirectory(core/errlib)
add_subdirectory(core/jstd)
add_subdirectory(resources)
add_subdirectory(pdflib)
if(BUILD_JAVA OR BUILD_PYTHON)
  add_subdirectory(bindings/api)
endif()
add_subdirectory(unittest EXCLUDE_FROM_ALL)





