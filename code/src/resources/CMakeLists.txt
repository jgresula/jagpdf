# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


# generate messages
jag_process_message_file(msg_resources)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(resources_SOURCES
  resfiles/icc_adobergb.cpp
  resfiles/icc_srgb.cpp
  imageman/imagemanimpl.cpp
  imageman/imagespecimpl.cpp
  imageman/imagepng.cpp
  imageman/imagejpeg.cpp
  imageman/imagefilterdata.cpp
  imageman/imagemaskimpl.cpp
  imageman/imagemanip.cpp
  typeman/typefacespecimpl.cpp
  typeman/typefaceutils.cpp
  typeman/typemanimpl.cpp
  typeman/fontspecimpl.cpp
  typeman/typefaceimpl.cpp
  typeman/freetypeopenargs.cpp
  typeman/fontimpl.cpp
  typeman/fontutils.cpp
  typeman/t1adobestandardfonts.cpp
  typeman/t1adobestandardface.cpp
  typeman/truetype/ttfont.cpp
  typeman/truetype/ttfontmaker.cpp
  typeman/truetype/ttfontparser.cpp
  typeman/truetype/ttstructs.cpp
  othermanagers/colorspacemanimpl.cpp
  othermanagers/colorspacesimpl.cpp
  resourcebox/defaultresourcectx.cpp
  resourcebox/resourcectximpl.cpp
  resourcebox/colorspacehelpers.cpp
  ${msg_resources_IMPL}
  )

if(UNIX)
  list(APPEND resources_SOURCES typeman/other/systemfontmapping.cpp)
elseif(WIN32)
  list(APPEND resources_SOURCES typeman/win32/systemfontmapping.cpp)
endif()

jag_setup_precompiled_headers(resources_SOURCES)
add_library(resources STATIC ${resources_SOURCES} ${msg_resources_HEADER})
add_dependencies(resources msg_errlib_TARGET msg_jstd_TARGET)

