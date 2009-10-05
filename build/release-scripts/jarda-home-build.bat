set PATH=c:/usr/lib;%PATH%

set BUILD_DIR=%1%
shift
set JAGPDF_DIST_DIR=%1%
shift
set JAGPDF_SOURCE_DIR=%1%
shift

cd "%BUILD_DIR%"
call "c:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat"
cmake -G "NMake Makefiles" ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_INSTALL_PREFIX=%JAGPDF_DIST_DIR% ^
-DPYTHON_INSTALL_DIR=%JAGPDF_DIST_DIR%/lib ^
-DFREETYPE_LIBRARY=c:/usr/src/freetype-2.3.9/objs/win32/vc2008/freetype239.lib ^
-DFREETYPE_INCLUDE_DIRS=c:/usr/src/freetype-2.3.9/include ^
-DZLIB_LIBRARY=c:/usr/src/zlib-1.2.3/projects/visualc6/Win32_DLL_Release/zlib1.lib ^
-DZLIB_INCLUDE_DIR=c:/usr/src/zlib-1.2.3 ^
-DPNG_LIBRARY=c:/usr/src/libpng-1.2.35/projects/visualc71/Win32_DLL_Release/libpng13.lib ^
-DPNG_PNG_INCLUDE_DIR=c:/usr/src/libpng-1.2.35 ^
-DICU_LIBRARY=c:/usr/src/icu-4.0.1/source/lib/icuuc.lib ^
-DICU_INCLUDE_DIR=c:/usr/src/icu-4.0.1/source/common ^
-DSWIG_EXECUTABLE=C:/usr/swigwin-1.3.39/swig.exe ^
-DBOOST_ROOT=c:/home/jarda/code/external/boost/boost_1_38_0 ^
-DBOOST_LIBRARYDIR=c:/home/jarda/code/external/boost/boost_1_38_0/lib ^
-DPYTHON_HOME=c:/programs/python26 ^
%* ../%JAGPDF_SOURCE_DIR%

if ERRORLEVEL 1 goto error

nmake
if ERRORLEVEL 1 goto error

nmake unit-tests
if ERRORLEVEL 1 goto error

nmake apitests
if ERRORLEVEL 1 goto error

nmake install
if ERRORLEVEL 1 goto error

exit 0

:error
echo NMake build error!
exit 1


