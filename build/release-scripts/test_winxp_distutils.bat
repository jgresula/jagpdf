@echo off
set python_dir=%1%
set jagpdf_dir=%2%
set virtualenv=%3%

set ENV_DIR=pyenv
rmdir /S /Q %ENV_DIR%
%python_dir%/python.exe %virtualenv% %ENV_DIR%
call %ENV_DIR%/Scripts/activate.bat
set PYEXEC=%ENV_DIR%\Scripts\python.exe
pushd %jagpdf_dir%
python setup.py install
popd

rmdir /S /Q  build
mkdir build
cd build
cmake -G "Unix Makefiles" ^
-DPYTHON_EXECUTABLE="../%PYEXEC%" ^
-DJAGPDF_PY_SYSTEM=ON ^
-DJAG_INSTALL_PREFIX=c:/nonexistent/dir/ ^
../apitest

make apitests

if ERRORLEVEL 1 goto error

exit 0

:error
echo Error!
exit 1



