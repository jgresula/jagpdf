# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


include(CTest)
SET (CTEST_CVS_CHECKOUT  "${CTEST_CVS_COMMAND} -d:pserver:hoffman@www.cmake.org:/cvsroot/CMake co -d\"${CTEST_SOURCE_DIRECTORY}\" CMake")
