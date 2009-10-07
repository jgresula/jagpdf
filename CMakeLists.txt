# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
set(CMAKE_COLOR_MAKEFILE false)

project(JagPDF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # On linux prefer /usr/local/ to /usr/.
  # By default, cmake searches /usr/ first which is questionable. See related discussion at
  # http://thread.gmane.org/gmane.comp.programming.tools.cmake.user/6683
  set(CMAKE_PREFIX_PATH /usr/local)
endif()


# 64-bit check
if(CMAKE_SIZEOF_VOID_P EQUAL 8 )
  if (WIN32)
    message(FATAL_ERROR "\nJagPDF has not been ported to 64-bit Windows.")
  endif()
endif()

# ---------------------------------------------------------------------------
#                           configuration
#                           
set(JAGPDF_VERSION_MAJOR 1)
set(JAGPDF_VERSION_MINOR 5)
set(JAGPDF_VERSION_PATCH 0)
set(JAGPDF_VERSION "${JAGPDF_VERSION_MAJOR}.${JAGPDF_VERSION_MINOR}.${JAGPDF_VERSION_PATCH}")

# ---------------------------------------------------------------------------
#                               options
#

# variables to simulate missing tools
option(JAG_WITH_PYTHON "internal" TRUE)
option(JAG_WITH_SWIG "internal" TRUE)
option(JAG_WITH_GCCXML "internal" TRUE)

# without python -> python bidings disabled, PDF comparison disabled, etc ..
# without swig -> only c/c++
# without gccxml -> all, but cannot alter api

#
# Options determining build type
#
set(BINARY_DISTRIBUTION OFF CACHE BOOL "Installation goes inside the build directory.")
set(ALL_IN_ONE OFF CACHE BOOL "Builds the all-in-one version of the library.")

if(ALL_IN_ONE)
  set(BINARY_DISTRIBUTION ON CACHE BOOL "Installation goes inside the build directory." FORCE)
endif()

if(BINARY_DISTRIBUTION)
  set(JAG_DIST_ACCOMPANYING_FILES
    ${CMAKE_SOURCE_DIR}/LICENSE.txt
    ${CMAKE_SOURCE_DIR}/CREDITS.txt
    ${CMAKE_SOURCE_DIR}/README.txt
    ${CMAKE_SOURCE_DIR}/INSTALL.txt
    ${CMAKE_SOURCE_DIR}/HISTORY.txt)
endif()

# the default build is release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are Release or Debug" FORCE)
endif()

set(REQ_BOOST_VERSION 1.36)
set(REQ_FREETYPE_VERSION 2.3.5)
set(REQ_ZLIB_VERSION 1.2.3)
# static version uses 1.2.25
set(REQ_PNG_VERSION 1.2.15)
# static version uses 3.8.1
set(REQ_ICU_VERSION 3.6.0) 

# eu-readelf ->
#  libc6 (>= 2.2.5)
#  libstdc++6 (>= 3.4)
#  
# version details:
#   http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
#   tilde ~ sorts before anything else
set(DEBIAN_PKG_DEP "libfreetype6 (>= ${REQ_FREETYPE_VERSION}), zlib1g (>= 1:${REQ_ZLIB_VERSION}), libpng12-0 (>= ${REQ_PNG_VERSION}~~), libicu38 (>= ${REQ_ICU_VERSION})")
set(DEBIAN_PKG_DEP "libc6 (>= 2.2.5), libstdc++6 (>= 3.4), ${DEBIAN_PKG_DEP}")




list(APPEND CMAKE_MODULE_PATH ${JagPDF_SOURCE_DIR}/build/cmake)
include(JagConfig)
include(JagCore)
include(JagTest)
include(JagPack)
if(PYTHON_HOME)
  list(INSERT CMAKE_PREFIX_PATH 0 "${PYTHON_HOME}")
endif()
if (JAG_WITH_PYTHON)
  find_package(PythonInterp 2.3 REQUIRED)
endif()
if(PYTHON_HOME)
  list(REMOVE_ITEM CMAKE_PREFIX_PATH "${PYTHON_HOME}")
endif()


#
# selection of components to build
#
macro(_SET_BOOL _var _value _comment)
   if (NOT DEFINED ${_var})
      set(${_var} ${_value})
   else (NOT DEFINED ${_var})
      set(${_var} "${${_var}}" CACHE BOOL "${_comment}")
   endif (NOT DEFINED ${_var})
endmacro()

_SET_BOOL(BUILD_C ON "Whether to build JagPDF for C/C++.")
_SET_BOOL(BUILD_PYTHON ON "Whether to build JagPDF for Python.")
_SET_BOOL(BUILD_JAVA ON "Whether to build JagPDF for Java.")
_SET_BOOL(DOCUMENTATION_ONLY OFF "Build only the documentation")

if(DOCUMENTATION_ONLY)
  set(BUILD_C OFF CACHE BOOL "" FORCE)
  set(BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  set(BUILD_JAVA OFF CACHE BOOL "" FORCE)
endif()

if(NOT PYTHON_EXECUTABLE)
  set(BUILD_PYTHON OFF CACHE BOOL "" FORCE)
endif()

if(NOT SWIG_FOUND)
  set(BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  set(BUILD_JAVA OFF CACHE BOOL "" FORCE)
endif()



#
# C/C++ options
#
set(INCLUDE_INSTALL_DIR "include/" CACHE STRING "Where to install JagPDF C/C++ headers.")
set(LIB_INSTALL_DIR "lib" CACHE STRING "Where to install JagPDF C/C++ library.")
if(BUILD_C)
  set(JAG_BUILD_C ON)
  set(JAG_C_LIB_LOCATION "${CMAKE_BINARY_DIR}/code/src/pdflib")
  set(JAG_C_INCLUDE_LOCATIONS "${CMAKE_BINARY_DIR}/include/" "${CMAKE_SOURCE_DIR}/code/include/")
else()
  set(JAG_BUILD_C OFF)
endif()

#
# Python options
# 
set(PYTHON_HOME "" CACHE STRING "Python directory.")
if(BUILD_PYTHON)
  if(PYTHON_HOME)
    list(INSERT CMAKE_PREFIX_PATH 0 "${PYTHON_HOME}")
  endif()
  find_package(PythonLibs)
  if(PYTHON_HOME)
    list(REMOVE_ITEM CMAKE_PREFIX_PATH "${PYTHON_HOME}")
  endif()
  if(PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
    # This is a work around a bug in FindPythonLibs.cmake which selects a static
    # Python library. See more at
    # http://www.vtk.org/Bug/view.php?id=8319
    string(REGEX REPLACE "^(.*/)python[0-9.]+/config/(libpython.*)\\.a$" "\\1\\2.so" PYTHON_SHARED ${PYTHON_LIBRARIES})
    if(PYTHON_SHARED)
      if(EXISTS ${PYTHON_SHARED})
        set(PYTHON_LIBRARIES ${PYTHON_SHARED})
      else()
        message(STATUS "WARNING: JagPDF for Python will be linked with static library.")
      endif()
    endif()
    # If PYTHON_INSTALL_DIR is specified then the files are copied to that
    # location and setup.py is not executed. Otherwise the standard distutils
    # installation is performed.
    set(PYTHON_INSTALL_DIR "/use-distutils" CACHE PATH "")
    if (PYTHON_INSTALL_DIR STREQUAL "/use-distutils")
      set(USE_DISTUTILS TRUE)
    else()
      set(USE_DISTUTILS FALSE)
    endif()
    set(JAG_PYTHON_LIB_LOCATION "${CMAKE_BINARY_DIR}/code/src/bindings/api/python/")
    set(JAG_BUILD_PYTHON ON)
    jag_exec_pycmd("import sys; print '%d%d' % sys.version_info[:2]" PY_MmVERSION)
    jag_exec_pycmd("import sys; print '%d.%d' % sys.version_info[:2]" PY_Mm_DOT_VERSION)
    jag_exec_pycmd("import sys; print sys.hexversion" PY_HEXVERSION)
  else()
    set(JAG_BUILD_PYTHON NOTFOUND)
  endif()
else()
  set(JAG_BUILD_PYTHON OFF)
endif()

#
# Java options
# 
set(JAVA_HOME "" CACHE STRING "Path to Java")
if(BUILD_JAVA)
  if(JAVA_HOME)
    set(ENV{JAVA_HOME} "${JAVA_HOME}")
    list(INSERT CMAKE_PREFIX_PATH 0 "${JAVA_HOME}")
  endif()
  find_package(Java)
  find_package(JNI)
  if(JAVA_HOME)
    list(REMOVE_ITEM CMAKE_PREFIX_PATH "${JAVA_HOME}")
  endif()
  if(NOT JAVA_INCLUDE_PATH)
    set(JAG_BUILD_JAVA NOTFOUND)
  else()
    set(JAGPDF_JAVA_JAR "jagpdf-${JAGPDF_VERSION_MAJOR}.${JAGPDF_VERSION_MINOR}.jar")
    set(JAG_JAVA_LIB_LOCATION "${CMAKE_BINARY_DIR}/code/src/bindings/api/java/")
    set(JAG_BUILD_JAVA ON)
  endif()
else()
  set(JAG_BUILD_JAVA OFF)
endif()


# ---------------------------------------------------------------------------
#                         common targets
#  
add_custom_target(dist-all)
add_custom_target(deb-all)


# ---------------------------------------------------------------------------
#                          testing
#                             
enable_testing()
# note: valgrind settings are in ${CMAKE_SOURCE_DIR}/CTestConfig.cmake
#set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/code/valgrind.supp")
set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/code/valgrind.supp" CACHE STRING "changed by me" FORCE)
set(MEMORYCHECK_COMMAND_OPTIONS 
  "-q --tool=memcheck --leak-check=yes --show-reachable=yes --workaround-gcc296-bugs=yes --num-callers=50" 
  CACHE STRING "my valgrind settings" FORCE)



# ---------------------------------------------------------------------------
#                           global variables
#                           
set(JAG_TOOLS_DIR ${CMAKE_SOURCE_DIR}/code/tools)


# ---------------------------------------------------------------------------
#                global defines, includes and configuration
#                           
add_definitions(-DBOOST_ALL_NO_LIB)      # disable boost autolink

# Debug configuration
message(STATUS "Build: ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DJAG_DEBUG)
  add_definitions(-D_DEBUG)            # e.g. gcc does not define this
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  add_definitions(-DNDEBUG)
endif()


# Windows only
if(WIN32 AND NOT CYGWIN)
  add_definitions(-DJAG_WIN32)
endif()

configure_file(
  "${CMAKE_SOURCE_DIR}/code/include/jagpdf/detail/version.h.in"
  "${CMAKE_BINARY_DIR}/include/jagpdf/detail/version.h")

configure_file(
  "${CMAKE_SOURCE_DIR}/code/include/jagpdf/detail/config.h.in"
  "${CMAKE_BINARY_DIR}/include/jagpdf/detail/config.h")

configure_file(
  "${CMAKE_SOURCE_DIR}/build/deb/deb-build.sh.in"
  "${CMAKE_BINARY_DIR}/deb/deb-build.sh"
  @ONLY)

# Globally disable questionable Microsoft warnings.
if(MSVC_VERSION)
  add_definitions(
    -D_CRT_SECURE_NO_DEPRECATE 
    -D_CRT_SECURE_NO_WARNINGS
    -D_SCL_SECURE_NO_WARNINGS
    -D_WIN32_WINNT=0x0500)
  # release only flags
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-D_SECURE_SCL=0)
  endif()
endif()


# TBD: dependency scanning
get_filename_component(_SRC_DIR_STEM ${CMAKE_SOURCE_DIR} NAME)
get_filename_component(_BUILD_DIR_STEM ${CMAKE_BINARY_DIR} NAME)
# message(SEND_ERROR ">> ${_SRC_DIR_STEM}")
# message(SEND_ERROR ">> ${_BUILD_DIR_STEM}")
#INCLUDE_REGULAR_EXPRESSION("^.*(${_SRC_DIR_STEM}|${_BUILD_DIR_STEM}).*")



# ---------------------------------------------------------------------------
#                               bitten
configure_file(
  ${CMAKE_SOURCE_DIR}/build/scripts/bitten_slave.sh
  ${CMAKE_BINARY_DIR}/bitten_slave.sh
  COPYONLY
)

macro(configure_bitten_ini_file STEM)
  configure_file(
    ${CMAKE_SOURCE_DIR}/build/scripts/${STEM}.ini.in
    ${CMAKE_BINARY_DIR}/${STEM}.ini
    @ONLY
    )
endmacro(configure_bitten_ini_file)

configure_bitten_ini_file("bitten_cmake_nightly_release")
configure_bitten_ini_file("bitten_cmake_nightly_debug")
configure_bitten_ini_file("bitten_cmake_smoke")


# ---------------------------------------------------------------------------
#                           subprojects
#                           
add_subdirectory(code/src)

if(EXISTS "${CMAKE_SOURCE_DIR}/code/test/apitest")
  add_subdirectory(code/test/apitest EXCLUDE_FROM_ALL)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/doc/quickbook)
  add_subdirectory(doc/quickbook EXCLUDE_FROM_ALL)
endif()

# ---------------------------------------------------------------------------
#                       configuration summary
#



if(DOCUMENTATION_ONLY)

else()
  message(STATUS "\n-----[ Configuration Summary ]---------------------\n")
  macro(print_lang_status var str)
    if("${var}" STREQUAL "")
      message(STATUS "${str}: OFF")
    else()
      message(STATUS "${str}: ${var}") 
    endif()
  endmacro()

  message(STATUS "Build JagPDF for C/C++: ${JAG_BUILD_C}")
  if (JAG_BUILD_C)
    message(STATUS " install include dir: ${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}")
    message(STATUS " install library dir: ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
  endif()
  
  message(STATUS "Build JagPDF for Python: ${JAG_BUILD_PYTHON}")
  if (JAG_BUILD_PYTHON)
    if(USE_DISTUTILS)
      message(STATUS " will use distutils")
    else()
      message(STATUS " install dir: ${PYTHON_INSTALL_DIR}")
    endif()
      message(STATUS " Python library: ${PYTHON_LIBRARIES}")
    message(STATUS " Python include: ${PYTHON_INCLUDE_PATH}")
  endif()

  message(STATUS "Build JagPDF for Java: ${JAG_BUILD_JAVA}")
  if (JAG_BUILD_JAVA)
    message(STATUS " install dir: ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
    message(STATUS " Java runtime: ${JAVA_RUNTIME}")
  endif()
endif()

  message(STATUS "-")
  
#
# Targets
# -------
#
# Installation
# ------------
# dist-c           ... installs JagPDF for C/C++
# dist-py          ... installs JagPDF for Python
# dist-java        ... installs JagPDF for Java
# dist-all         ... all above

# Testing
# -------
# unit-tests       ... builds and runs internal unit tests
# apitests         ... all apitests 
# apitests-cpp     ... builds and runs C++ api tests (needs install-jagpdf)
# apitests-c       ... builds and runs C api tests (needs install-jagpdf)
# apitests-py      ... runs python tests
# 

