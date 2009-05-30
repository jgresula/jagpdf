# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# generate messages
jag_process_message_file(msg_jstd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(jstd_SOURCES
  encodinghelpers.cpp
  execcontextimpl.cpp
  unicode.cpp
  uconverterctrl.cpp
  message_sink_console.cpp
  md5.cpp
  arcfour_stream.cpp
  file_stream.cpp
  memory_stream.cpp
  zlib_stream.cpp
  streamhelpers.cpp
  configimpl.cpp
  optionsparser.cpp
  stringpool.cpp
  tracer.cpp
  msg_jstd.jmsg
  crt.cpp
  tssgeneric.cpp
  conversions.cpp
  transaffine.cpp
  icumain.cpp
  ${msg_jstd_IMPL})


# form a list of platform dependent sources
set(_platform_SOURCES
  fileso.cpp
  tss.cpp
  dynlib.cpp
  thread.cpp
  mmap.cpp)

if(UNIX)
  set(PLATFORM_DIR "other")
elseif(WIN32)
  list(APPEND _platform_SOURCES win32common.cpp)
  set(PLATFORM_DIR "win32")
endif()
foreach(SRC ${_platform_SOURCES})
  list(APPEND jstd_SOURCES "${PLATFORM_DIR}/${SRC}")
endforeach()

jag_setup_precompiled_headers(jstd_SOURCES)
add_library(
  jstd
  STATIC
  # sources
  ${jstd_SOURCES}
  # dependency
  ${msg_jstd_HEADER}
  )
add_dependencies(jstd msg_errlib_TARGET)


