# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


jag_process_message_file(msg_errlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(errlib_SOURCES
  messages.cpp
  log.cpp
  except.cpp
  ${msg_errlib_IMPL})

jag_setup_precompiled_headers(errlib_SOURCES)
add_library(errlib STATIC ${errlib_SOURCES} ${msg_errlib_HEADER})
