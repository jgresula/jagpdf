# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


include(${SWIG_USE_FILE})

include_directories(${API_SWIG_DIR})
include_directories(${CURRENT_SOURCE_DIR})
add_definitions(-DJAGAPI_BUILDING_CPP)

if(MSVC_VERSION)
  string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif(JAG_GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
endif()

list(APPEND SwigFlags
  "-I${CMAKE_SOURCE_DIR}/code/src/bindings/swiglib"
  -DJAG_EXPORT
  -DJAG_CALLSPEC
  )

# ---------------------------------------------------------------------------
#                           subprojects
#
if(JAG_BUILD_PYTHON)
  add_subdirectory(python)
endif()

if(JAG_BUILD_JAVA)
  add_subdirectory(java)
endif()
