# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(Dart)
set(CTEST_PROJECT_NAME "JagPDF")

set(CTEST_NIGHTLY_START_TIME "23:00:00 UTC")
set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "192.168.1.162")
set(CTEST_DROP_LOCATION "/cdash/submit.php?project=JagPDF")
set(CTEST_DROP_SITE_CDASH TRUE)
set(CTEST_UPDATE_TYPE svn)