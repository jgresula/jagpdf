# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


#
# Variables
#
if(WIN32)
  set(JAG_SHARED_LIB_EXTENSION ".dll")
  set(JAG_IMPORT_LIB_EXTENSION ".lib")
else()
  set(JAG_SHARED_LIB_EXTENSION ".so")
  set(JAG_IMPORT_LIB_EXTENSION JAG_IMPORT_LIB_EXTENSION-NOTFOUND)
endif()

if(WIN32)
  set(PATH_SEP "\;")
else()
  set(PATH_SEP ":")
endif()

#
# Executes single python command and returns stdout
#
macro(jag_exec_pycmd CMD OUTVAR)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "${CMD}"
    OUTPUT_VARIABLE _PYOUT
    )
  STRING(STRIP ${_PYOUT} ${OUTVAR})
endmacro()


#
# joins a list, individual items are separated by SEP
# 
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


#
# Appends the  value of SRC_PROPERTY to source file FILE
#
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


#
# generates .h and .cpp files from .jmsg
#
# defines variables:
#  - ${STEM}_HEADER  .. generated header file
#  - ${STEM}_IMPL    .. generated implementation file
#  - ${STEM}_TARGET  .. target generating the files
#  
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/messages)
macro(jag_process_message_file STEM)
  set(MSG_HEADER ${MSG_INCLUDE_DIR}/${STEM}.h)
  set(MSG_IMPL ${CMAKE_CURRENT_BINARY_DIR}/${STEM}.cpp)
  set(MSG_DEF ${CMAKE_CURRENT_SOURCE_DIR}/${STEM}.jmsg)

  set(PREBUILD_MSG_DIR "${CMAKE_SOURCE_DIR}/code/src/generated/messages")
  if (PYTHON_EXECUTABLE)
    add_custom_command(
      OUTPUT ${MSG_HEADER} ${MSG_IMPL}
      DEPENDS ${MSG_DEF}
      COMMAND ${PYTHON_EXECUTABLE} ${JAG_TOOLS_DIR}/msggen.py ${MSG_DEF} ${MSG_IMPL} ${MSG_HEADER}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MSG_HEADER}" "${PREBUILD_MSG_DIR}/${STEM}.h.in"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MSG_IMPL}" "${PREBUILD_MSG_DIR}/${STEM}.cpp.in")
  else()
    # use the prebuilt ones
    configure_file("${PREBUILD_MSG_DIR}/${STEM}.h.in" "${MSG_HEADER}" COPYONLY)
    configure_file("${PREBUILD_MSG_DIR}/${STEM}.cpp.in" "${MSG_IMPL}" COPYONLY)
  endif()

  set(${STEM}_HEADER ${MSG_HEADER})
  set(${STEM}_IMPL ${MSG_IMPL})
  add_custom_target(
    ${STEM}_TARGET
    DEPENDS ${MSG_HEADER} ${MSG_IMPL})

endmacro(jag_process_message_file)


#
# generates FNAME.rc file with description DESC
#
# defines variable
#  - ${${FNAME}_RC_FILE}  .. generated rc file
#  
macro(jag_create_rc_file FNAME DESC)
  if(MSVC_VERSION)
    set(${FNAME}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${FNAME}.rc")
    set(JAGPDF_RC_DESCRIPTION "${DESC}")
    configure_file(${CMAKE_SOURCE_DIR}/code/src/pdflib/jagpdf.rc.in ${${FNAME}_RC_FILE})
    # the .rc file is windows-1252 encoded
    jag_append_source_flag(${${FNAME}_RC_FILE} COMPILE_FLAGS "/c 1252")
  endif()
endmacro(jag_create_rc_file)



#
# Implements support for precompiled headers. Assumes that precompiled.cpp and
# precompiled.h are in the current source directory.
#
# It updates list of sources and also their compilation flags.
#
# SOURCES .. name of the variable holding list of sources
# 
macro(jag_setup_precompiled_headers SOURCES)
  if(NOT ${SOURCES})
    # if the line above fails with syntax error then you are likely passing list
    # of sources instead of the name of the variable that holds sources
  endif(NOT ${SOURCES})
  if(JAG_PRECOMPILED_HEADERS)
    if (MSVC_VERSION)
      set(THIS_PCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/precompiled.pch")
      set(JAG_PRECOMPILED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.cpp")
      # set properties for the precompiled file
      jag_append_source_flag(${JAG_PRECOMPILED_HEADER} COMPILE_FLAGS "/Ycprecompiled.h /Fp${THIS_PCH_FILE}")
      jag_append_source_flag(${JAG_PRECOMPILED_HEADER} OBJECT_OUTPUTS "${THIS_PCH_FILE}")
      jag_append_source_flag(${JAG_PRECOMPILED_HEADER} COMPILE_DEFINITIONS "JAG_PRECOMPILED_HEADERS")
      # set properties for sources
      foreach(SRC ${${SOURCES}})
        jag_append_source_flag(${SRC} COMPILE_FLAGS "/Yuprecompiled.h /Fp${THIS_PCH_FILE}")
        jag_append_source_flag(${SRC} OBJECT_DEPENDS ${THIS_PCH_FILE})
        jag_append_source_flag(${SRC} COMPILE_DEFINITIONS "JAG_PRECOMPILED_HEADERS")
      endforeach(SRC)
      list(APPEND ${SOURCES} ${JAG_PRECOMPILED_HEADER})
    endif(MSVC_VERSION)
  endif(JAG_PRECOMPILED_HEADERS)
endmacro(jag_setup_precompiled_headers)

