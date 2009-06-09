# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#include(FindThreads)
# if (NOT CMAKE_THREAD_LIBS_INIT)
#   message(FATAL_ERROR "Thread library not found")
# endif()

# sets JAG_HAVE_MEMCHECK to 'memcheck' if in Linux (should be more sophisticated
# test)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(JAG_HAVE_MEMCHECK "memcheck")
else()
  set(JAG_HAVE_MEMCHECK "")
endif()

#---------------------------------------------------------------------------
#                         COMPILER VARIABLES
#
# MSVC
# ----
#  - MSVC_VERSION <- _MSC_VER
#  - MSVCnn .. nn is the compiler version, e,g MSVC71
#  - MSVC_MAJOR .. major version
#  - MSVC_MINOR .. minor version
#  - JAG_GCCXML_COMPILER .. compiler to be emulated by gccxml
#  - here is the mapping from cmake MSVC_VERSION (cl _MSC_VER)
#      MSVC50 .. _MSC_VER < 1200
#      MSVC60 .. _MSC_VER < 1300
#      MSVC70 .. _MSC_VER == 1300
#      MSVC71 .. _MSC_VER == 1310
#      MSVC80 .. _MSC_VER == 1400
#      MSVC90 .. _MSC_VER == 1500
# GCC
# ---
#  - JAG_GCC_IN_UNIX ... TRUE if gcc in Linux
#  - JAG_GCC_VERSION_STRING ... gcc version triplet
#  - JAG_GCC_LIBSTDCPP_DIR ... location of libstdc++
# 
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
  set(JAG_GCC TRUE)
  if (UNIX)
    set(JAG_GCC_IN_UNIX TRUE)
  endif()
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} "-dumpversion" 
    OUTPUT_VARIABLE JAG_GCC_VERSION_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (BINARY_DISTRIBUTION)
    execute_process(
      #COMMAND dirname "`${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so`"
      COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so
      OUTPUT_VARIABLE JAG_GCC_LIBSTDCPP_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    GET_FILENAME_COMPONENT(JAG_GCC_LIBSTDCPP_DIR ${JAG_GCC_LIBSTDCPP_DIR} PATH)
  endif()
elseif(MSVC60)
  set(MSVC_MAJOR 6)
  set(MSVC_MINOR 0)
  set(JAG_GCCXML_COMPILER msvc6)
elseif(MSVC70)
  set(MSVC_MAJOR 7)
  set(MSVC_MINOR 0)
  set(JAG_GCCXML_COMPILER msvc7)
elseif(MSVC71)
  set(MSVC_MAJOR 7)
  set(MSVC_MINOR 1)
  set(JAG_GCCXML_COMPILER msvc71)
elseif(MSVC80)
  set(MSVC_MAJOR 8)
  set(MSVC_MINOR 0)
  set(JAG_GCCXML_COMPILER msvc8)
elseif(MSVC90)
  set(MSVC_MAJOR 9)
  set(MSVC_MINOR 0)
  set(JAG_GCCXML_COMPILER msvc9)
endif()

if(JAG_GCC)
  message(STATUS "gcc version: ${JAG_GCC_VERSION_STRING}")
elseif(MSVC_MAJOR)
  message(STATUS "msvc version: ${MSVC_VERSION}")  
endif()



#---------------------------------------------------------------------------
#                             GCCXML
#
if(JAG_WITH_GCCXML)
  set(REQ_GCCXML_VERSION 0.9.0)
  find_package(GCCXML REQUIRED)
endif()

if(GCCXML)
  execute_process(
    COMMAND "${GCCXML}" "--version"
    OUTPUT_VARIABLE GCCXML_VERSION)

  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" GCCXML_VERSION ${GCCXML_VERSION})
  if(GCCXML_VERSION VERSION_LESS REQ_GCCXML_VERSION)
    set(GCCXML NOTFOUND CACHE FILEPATH "Path to gccxml compiler." FORCE)
    message(FATAL_ERROR "GCCXML version ${GCCXML_VERSION} found, but at least ${REQ_GCCXML_VERSION} is required")
  endif()
  message(STATUS "GCCXML version: ${GCCXML_VERSION}")

  macro(get_gccxml_flags _GCCXML_FLAGS)
    get_directory_property(
      _GCCXML_INCLUDES
      DIRECTORY "${CMAKE_SOURCE_DIR}/code/src/pdflib"
      INCLUDE_DIRECTORIES)
    
    set(_GCCXML_INCLUDE_ARG)
    foreach(INC_DIR ${_GCCXML_INCLUDES})
      list(APPEND _GCCXML_INCLUDE_ARG "-I${INC_DIR}")
    endforeach(INC_DIR)

    set(_GCCXML_COMPILER_FLAGS "--gccxml-path=${GCCXML}" "--gccxml-cache=${CMAKE_BINARY_DIR}/gcccache")
    if(JAG_GCCXML_COMPILER)
      list(APPEND _GCCXML_COMPILER_FLAGS "--gccxml-compiler=${JAG_GCCXML_COMPILER}")
    endif()

    list(APPEND _RESULT ${_GCCXML_INCLUDE_ARG})
    list(APPEND _RESULT ${_GCCXML_COMPILER_FLAGS})
    set(${_GCCXML_FLAGS} ${_RESULT})
    # even though gccxml is available we need to check that it actually works
    if (NOT GCCXML_TESTED)
      execute_process(
        COMMAND 
        "${GCCXML}" ${_GCCXML_INCLUDE_ARG} 
        --gccxml-compiler 
        ${JAG_GCCXML_COMPILER} 
        ${CMAKE_SOURCE_DIR}/code/include/pdflib/interfaces/canvas.h
        RESULT_VARIABLE GCCXML_EXIT)
      set(GCCXML_TESTED TRUE CACHE INTERNAL "whether gccxml santity check has been run")
      if(NOT GCCXML_EXIT EQUAL 0)
        message(STATUS "GCCXML does not work, disabling.")
        set(GCCXML OFF)
      endif()
    endif()
  endmacro()
else()
  message(STATUS "GCCXML not found.")
endif()

# ---------------------------------------------------------------------------
#                   SWIG
#
if(JAG_WITH_SWIG)
  if(ALL_IN_ONE)
    # SWIG
    if(WIN32)
      set(SWIG_EXECUTABLE ${CMAKE_SOURCE_DIR}/external/tools/swig-1.3.33/swig.exe)
    else()
      # in linux the path to the library is hard-wired into the executable; thus
      # we encapsulate the swig binary with a script which sets SWIG_LIBRARY env
      # variable pointing to our copy of the swig library
      set(SWIG_EXECUTABLE "${CMAKE_BINARY_DIR}/tools/swig.sh")
      set(JAG_SWIG_LIB "${CMAKE_SOURCE_DIR}/external/tools/swig-1.3.33/Lib")
      configure_file("${CMAKE_SOURCE_DIR}/external/tools/swig-1.3.33/swig.sh.in" "${SWIG_EXECUTABLE}")
    endif()
  endif()
  set(REQ_SWIG_VERSION 1.3.33)
  FIND_PACKAGE(SWIG ${REQ_SWIG_VERSION})
endif()


#---------------------------------------------------------------------------
#                         VERSION CHECKERS
#
# some cmake modules do not implement version checking

# FreeType
#
# FREETYPE_VERSION
# 
macro(check_freetype_version req_version incdir)
  find_file(_freetype_h "freetype/freetype.h" PATHS ${incdir})
  if(NOT _freetype_h)
    message(FATAL_ERROR "Could not find freetype/freetype.h")
  endif()
  file(READ "${_freetype_h}" _freetype_h_contents)
  STRING(REGEX REPLACE ".*#define FREETYPE_MAJOR +([0-9]+).*" "\\1" FREETYPE_MAJOR "${_freetype_h_contents}")
  STRING(REGEX REPLACE ".*#define FREETYPE_MINOR +([0-9]+).*" "\\1" FREETYPE_MINOR "${_freetype_h_contents}")
  STRING(REGEX REPLACE ".*#define FREETYPE_PATCH +([0-9]+).*" "\\1" FREETYPE_PATCH "${_freetype_h_contents}")
  set(FREETYPE_VERSION "${FREETYPE_MAJOR}.${FREETYPE_MINOR}.${FREETYPE_PATCH}")
  message(STATUS "FreeType version: ${FREETYPE_VERSION}")
  if (${FREETYPE_VERSION} VERSION_LESS ${req_version})
    message(SEND_ERROR "Freetype version ${req_version} required.")
  endif()
endmacro(check_freetype_version)


# Zlib
#
# ZLIB_VERSION
# 
macro(check_zlib_version req_version incdir)
  find_file(_zlib_h "zlib.h" PATHS ${incdir})
  if(NOT _zlib_h)
    message(FATAL_ERROR "Could not find zlib.h")
  endif()
  file(READ "${_zlib_h}" _zlib_h_contents)
  STRING(REGEX REPLACE ".*#define ZLIB_VERSION \"+([0-9.]+)\".*" "\\1" ZLIB_VERSION "${_zlib_h_contents}")
  message(STATUS "ZLIB version: ${ZLIB_VERSION}")
  if (${ZLIB_VERSION} VERSION_LESS ${req_version})
    message(SEND_ERROR "ZLIB version ${req_version} required.")
  endif()
endmacro(check_zlib_version)


# PNG
#
# PNG_VERSION
# 
macro(check_png_version req_version incdir)
  find_file(_png_h "png.h" PATHS ${incdir})
  if(NOT _png_h)
    message(FATAL_ERROR "Could not find png.h")
  endif()
  file(READ "${_png_h}" _png_h_contents)
  STRING(REGEX REPLACE ".*#define PNG_LIBPNG_VER_MAJOR +([0-9]+).*" "\\1" PNG_MAJOR "${_png_h_contents}")
  STRING(REGEX REPLACE ".*#define PNG_LIBPNG_VER_MINOR +([0-9]+).*" "\\1" PNG_MINOR "${_png_h_contents}")
  STRING(REGEX REPLACE ".*#define PNG_LIBPNG_VER_RELEASE +([0-9]+).*" "\\1" PNG_PATCH "${_png_h_contents}")
  set(PNG_VERSION "${PNG_MAJOR}.${PNG_MINOR}.${PNG_PATCH}")
  message(STATUS "PNG version: ${PNG_VERSION}")
  if (${PNG_VERSION} VERSION_LESS ${req_version})
    message(SEND_ERROR "PNG version ${req_version} required.")
  endif()
endmacro(check_png_version)


# ICU
#
# ICU_VERSION
# 
macro(check_icu_version req_version incdir)
  find_file(_uversion_h "unicode/uversion.h" PATHS ${incdir})
  if(NOT _uversion_h)
    message(FATAL_ERROR "unicode/uversion.h")
  endif()
  file(READ "${_uversion_h}" _uversion_h_contents)
  STRING(REGEX REPLACE ".*#define U_ICU_VERSION_MAJOR_NUM +([0-9]+).*" "\\1" ICU_MAJOR "${_uversion_h_contents}")
  STRING(REGEX REPLACE ".*#define U_ICU_VERSION_MINOR_NUM +([0-9]+).*" "\\1" ICU_MINOR "${_uversion_h_contents}")
  STRING(REGEX REPLACE ".*#define U_ICU_VERSION_PATCHLEVEL_NUM +([0-9]+).*" "\\1" ICU_PATCH "${_uversion_h_contents}")
  set(ICU_VERSION "${ICU_MAJOR}.${ICU_MINOR}.${ICU_PATCH}")
  message(STATUS "ICU version: ${ICU_VERSION}")
  if (${ICU_VERSION} VERSION_LESS ${req_version})
    message(SEND_ERROR "ICU version ${req_version} required.")
  endif()
endmacro(check_icu_version)


# checks that libxml2 for python is available
#
# - It is questionable whether to use the configured python interpreter or the
#   one in the system path. Probably it should be the configured one, as there
#   might be no one.
#   
macro(check_libxml_python)
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import libxml2"
    RESULT_VARIABLE LIBXML2_CODE)
  if (NOT LIBXML2_CODE EQUAL 0)
    message(FATAL_ERROR "Libxml2 for python not found.")
  endif()
endmacro(check_libxml_python)




