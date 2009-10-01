# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# TBD
#
# - some macros redefined here (jag_list_join, ...)
#
# - python detection differs from non-standalone mode (CMAKE_PREXIF_PATH is
#   altered in the latter case) -> would be solved by sharing cmake code (see
#   above)
#
get_directory_property(MY_PARENT_DIR PARENT_DIRECTORY)

# empty parent directory indicates that this is the top directory and thus we
# are in standalone apitests mode
if(NOT MY_PARENT_DIR)
  find_package(PythonInterp REQUIRED 2.3)
  message(STATUS "API tests standalone mode.")
  set(JAG_STANDALONE_APITESTS 1)
endif()

if(JAG_STANDALONE_APITESTS)
  # ---------------------------------------------------------------------------
  #                         standalone apitests
  #

  # JAVA on linux
  # -------------
  #  - download self-extracting binary, chmod +x, and run it
  #  - it creates a dir in the current dir
  #
  #
  # cmake configuration
  # -------------------
  # cmake ~/code/jagbase/code/test/apitest/
  #       -DJAG_INSTALL_PREFIX=/home/jarda/tmp/apit/distribution/
  #       -DPYTHON_EXECUTABLE=/usr/local/python/2.5.2/bin/python
  #       -DCMAKE_PREFIX_PATH=/home/jarda/src/j2sdk1.4.2_18/

  cmake_minimum_required(VERSION 2.6)
  set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
  #set(CMAKE_VERBOSE_MAKEFILE TRUE)

  enable_testing()
  set(JAG_TEST_ENV "${CMAKE_BINARY_DIR}/CTestInclude.cmake")

  #
  # setup paths to be searched for JagPDF
  #
  if(NOT JAG_JAVA_LIB_LOCATION)
    set(JAG_JAVA_LIB_LOCATION "${JAG_INSTALL_PREFIX}/lib")
  endif()
  file(TO_CMAKE_PATH ${JAG_JAVA_LIB_LOCATION} JAG_JAVA_LIB_LOCATION)

  if(NOT JAG_PYTHON_LIB_LOCATION)
    set(JAG_PYTHON_LIB_LOCATION "${JAG_INSTALL_PREFIX}/lib")
  endif()
  file(TO_CMAKE_PATH ${JAG_PYTHON_LIB_LOCATION} JAG_PYTHON_LIB_LOCATION)

  if(NOT JAG_C_INCLUDE_LOCATIONS)
    set(JAG_C_INCLUDE_LOCATIONS "${JAG_INSTALL_PREFIX}/include")
  endif()
  file(TO_CMAKE_PATH ${JAG_C_INCLUDE_LOCATIONS} JAG_C_INCLUDE_LOCATIONS)

  if(NOT JAG_C_LIB_LOCATION)
    set(JAG_C_LIB_LOCATION "${JAG_INSTALL_PREFIX}/lib")
  endif()
  file(TO_CMAKE_PATH ${JAG_C_LIB_LOCATION} JAG_C_LIB_LOCATION)

  #
  # find out JagPDF versions
  #
  if (IS_DIRECTORY "${JAG_C_INCLUDE_LOCATIONS}")
    message(STATUS "Found JagPDF for C/C++")
    set(JAG_APITEST_C 1)
    if (WIN32)
      file(GLOB JAGPDF_IMPORT_LIB "${JAG_C_LIB_LOCATION}" "${JAG_C_LIB_LOCATION}/jagpdf*.lib")
      string(REGEX MATCH "jagpdf-[0-9]+\\.[0-9]+" jagpdf-bin-outname ${JAGPDF_IMPORT_LIB})
    else()
      set(jagpdf-bin-outname jagpdf)
    endif()
  endif()

  if(JAGPDF_PY_SYSTEM OR EXISTS "${JAG_PYTHON_LIB_LOCATION}/jagpdf.py")
    message(STATUS "Found JagPDF for Python")
    set(JAG_APITEST_PY 1)
  endif()

  file(GLOB JAGPDF_JAVA_JAR RELATIVE "${JAG_JAVA_LIB_LOCATION}" "${JAG_JAVA_LIB_LOCATION}/jagpdf-*.jar")
  if(JAGPDF_JAVA_JAR)
    list(LENGTH JAGPDF_JAVA_JAR NUM_JAR_FILES)
    if (NOT NUM_JAR_FILES EQUAL 1)
      message(FATAL_ERROR "More then one *.jar files found")
    endif()
    message(STATUS "Found JagPDF for Java")
    set(JAG_APITEST_JAVA 1)
  endif()

  if (NOT JAG_APITEST_PY AND NOT JAG_APITEST_C AND NOT JAG_APITEST_JAVA)
    message(FATAL_ERROR "JagPDF not found. Make sure that JAG_INSTALL_PREFIX is set correctly.")
  endif()

  # do not configure C/C++ compiler if JagPDF for C/C++ is not found
  # by passing NONE to project(); this is an undocumented feature, see
  # http://thread.gmane.org/gmane.comp.programming.tools.cmake.user/15576/focus=15579
  #
  if(JAG_APITEST_C)
    PROJECT(apitests)
  else()
    PROJECT(apitest NONE)
  endif()

# ---------------------------------------------
#  temporarily
if(WIN32)
  set(PATH_SEP "\;")
else()
  set(PATH_SEP ":")
endif()

macro(jag_list_join OUTVAR LIST SEP)
  set(RESULT "")
  foreach(ITEM ${LIST})
    if(RESULT)
      set(RESULT "${RESULT}${SEP}${ITEM}")
    else()
      set(RESULT "${ITEM}")
    endif()
  endforeach(ITEM)
  set(${OUTVAR} ${RESULT})
endmacro(jag_list_join)

macro(jag_append_source_flag FILE SRC_PROPERTY ...)
  get_source_file_property(TMP_SRC_FLAG ${FILE} ${SRC_PROPERTY})
  set(VALUES ${ARGV})
  list(REMOVE_AT VALUES 0 1)
  foreach(FLAG ${VALUES})
    if (TMP_SRC_FLAG)
      set(TMP_SRC_FLAG "${TMP_SRC_FLAG} ${FLAG}")
    else()
      set(TMP_SRC_FLAG "${FLAG}")
    endif(TMP_SRC_FLAG)
  endforeach(FLAG)
  set_source_files_properties(${FILE} PROPERTIES ${SRC_PROPERTY} "${TMP_SRC_FLAG}")
endmacro(jag_append_source_flag)

# end temporarily
# -------------------------------------------------

else()

  # ---------------------------------------------------------------------------
  #                         build tree apitests
  #
  set(JAG_TEST_ENV "${CMAKE_BINARY_DIR}/CTestInclude.cmake")
  get_target_property(jagpdf-bin-outname jagpdf OUTPUT_NAME)

endif()



if(WIN32)
  set(JAG_MYCTEST_SH "${CMAKE_CTEST_COMMAND}")
else()
  set(JAG_MYCTEST_SH "${CMAKE_CURRENT_SOURCE_DIR}/tools/myctest.sh")
endif()

if(PYTHON_EXECUTABLE)
  set(JAG_COMPARE_PDF ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/compare_pdf.py)
else()
  set(JAG_COMPARE_PDF echo "Skipping PDF comparison, python is required.")
endif()


#
# targets
#
add_custom_target(apitests)
add_custom_target(apitests-memcheck)



#
# Variables
#
set(PDF_REFERENCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/reference-pdf)


#
# Conditionally include subdirs with tests for individual languages
#

# C/C++
#
if(JAG_BUILD_C OR JAG_APITEST_C)
  # define imported jagpdf-bin-c project - JagPDF for C/C++
  add_library(jagpdf-bin-c SHARED IMPORTED)
  if(WIN32)
    set_target_properties(jagpdf-bin-c
      PROPERTIES
      IMPORTED_IMPLIB ${JAG_C_LIB_LOCATION}/${jagpdf-bin-outname}.lib
      IMPORTED_LOCATION ${JAG_C_LIB_LOCATION}/${jagpdf-bin-outname}.dll
      )
  else()
    set_target_properties(jagpdf-bin-c
      PROPERTIES
      IMPORTED_LOCATION ${JAG_C_LIB_LOCATION}/lib${jagpdf-bin-outname}.so
      )
  endif()
  # preamble for c and c++ tests
  macro(c_cpp_test_preamble PDFOutDir)
    set(CXX_TEST_PATH ${CMAKE_CURRENT_BINARY_DIR})
    set(TEST_PDF_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${PDFOutDir})
    file(MAKE_DIRECTORY ${TEST_PDF_OUTPUT_DIR})
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILE ${JAG_TEST_ENV})
    # compiler settings
    include_directories(${JAG_C_INCLUDE_LOCATIONS})
  endmacro(c_cpp_test_preamble)
  # include c/c++ subdirs
  add_subdirectory(cpp EXCLUDE_FROM_ALL)
  add_subdirectory(c EXCLUDE_FROM_ALL)
endif()

# Python
#
if(JAG_BUILD_PYTHON OR JAG_APITEST_PY)
  add_subdirectory(py EXCLUDE_FROM_ALL)
endif()

# Java
#
if(JAG_BUILD_JAVA OR JAG_APITEST_JAVA)
  add_subdirectory(java EXCLUDE_FROM_ALL)
endif()


# ---------------------------------------------------------------------------
#                           test paths
#
# the test environment is setup through ${JAG_TEST_ENV}

#
# non-install testing
#
file(WRITE ${JAG_TEST_ENV}
  "set(ENV{JAG_TEST_RESOURCES_DIR} \"${CMAKE_CURRENT_SOURCE_DIR}/resources\")\n")
# PYTHONPATH
if(JAG_PYTHON_LIB_LOCATION)
  file(APPEND ${JAG_TEST_ENV} "set(ENV{PYTHONPATH} \"${JAG_PYTHON_LIB_LOCATION}\")\n")
endif()
# LD_LIBRARY_PATH/PATH
set(TEST_LIB_PATH "")
list(APPEND TEST_LIB_PATH
  ${JAG_GCC_LIBSTDCPP_DIR}
  ${JAG_JAVA_LIB_LOCATION}
  ${JAG_C_LIB_LOCATION})
list(REMOVE_DUPLICATES TEST_LIB_PATH)
jag_list_join(TEST_LIB_PATH_JOINED "${TEST_LIB_PATH}" ${PATH_SEP})
if(WIN32)
  set(OS_LD_PATH "PATH")
  file(TO_CMAKE_PATH "$ENV{PATH}" _ENV_LD)
else()
  set(OS_LD_PATH "LD_LIBRARY_PATH")
  set(_ENV_LD "$ENV{LD_LIBRARY_PATH}")
endif()
file(APPEND ${JAG_TEST_ENV} "set(ENV{${OS_LD_PATH}} \"${TEST_LIB_PATH_JOINED}${PATH_SEP}${_ENV_LD}\")\n")

