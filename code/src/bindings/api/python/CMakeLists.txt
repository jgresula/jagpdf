# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

include_directories(${PYTHON_INCLUDE_PATH})

list(APPEND SwigFlags
  # module name added manually - solves re-builts as well
  -module jagpdf
  )

set_source_files_properties(jagpdf.i
  PROPERTIES
  CPLUSPLUS ON
  SWIG_FLAGS "${SwigFlags}"
)

SET(SWIG_MODULE_jagpdf_EXTRA_DEPS
  ../files.swg
  ${API_SWIG_DIR}/generated.swg)

# Python
jag_create_rc_file(pyjagpdf "JagPDF extension for Python.")
swig_add_module(jagpdf python jagpdf.i ../swigdllmain.cpp ${pyjagpdf_RC_FILE})
swig_link_libraries(jagpdf ${PYTHON_LIBRARIES} pdflib-static-core)

if (JAG_GCC_IN_UNIX)
  set_target_properties(_jagpdf
    PROPERTIES
    LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.gcc.map ${LIBSTDCPP_LINK_FLAG} -ldl")
elseif(MSVC_VERSION)
  set_target_properties(_jagpdf PROPERTIES SUFFIX ".pyd")
endif()

#
# Installation definition.
#

if (USE_DISTUTILS)
  SET(SETUP_PY_PKG_DIR "")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/distutils.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/distutils.cmake" 
    @ONLY)
  
  install(
    SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/distutils.cmake
    COMPONENT pyjagpdf)
else()
  # Python installation path needs to relativized, otherwise cpack ignores it. The
  # trailing '/.' is important, because if the two paths are equal then the result
  # is an empty variable, which confuses the install command
  file(RELATIVE_PATH PYTHON_INSTALL_RELDIR ${CMAKE_INSTALL_PREFIX} ${PYTHON_INSTALL_DIR}/.)

  install( # extension
    TARGETS _jagpdf
    DESTINATION ${PYTHON_INSTALL_RELDIR}
    COMPONENT pyjagpdf)

  install( # python file
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jagpdf.py
    DESTINATION ${PYTHON_INSTALL_RELDIR}
    COMPONENT pyjagpdf)


  install( # other files
    FILES
    ${JAG_DIST_ACCOMPANYING_FILES}
    DESTINATION "${PYTHON_INSTALL_RELDIR}/.."
    COMPONENT pyjagpdf)

  install( # setup.py - requires executable permissions
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py
    PERMISSIONS 
    OWNER_EXECUTE OWNER_READ OWNER_WRITE
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
    DESTINATION "${PYTHON_INSTALL_RELDIR}/.."
    COMPONENT pyjagpdf)

  SET(SETUP_PY_PKG_DIR "lib")

  ADD_COMPONENT_PACKAGE(pyjagpdf py${PY_MmVERSION} python${PY_MmVERSION}-jagpdf)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in"
  "${CMAKE_CURRENT_BINARY_DIR}/setup.py"
  @ONLY)


#
# Installs jagpdf for python
#
add_custom_target(dist-py
  COMMAND ${CMAKE_COMMAND}
  -D "CMAKE_INSTALL_COMPONENT=pyjagpdf"
  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
  )
add_dependencies(dist-py _jagpdf)
add_dependencies(dist-all dist-py)

#
# Checks that the produced binary is ok
#
add_custom_target(check-pyjagpdf-binaries
  COMMAND ${PYTHON_EXECUTABLE} ${JAG_TOOLS_DIR}/check_package.py ${CMAKE_CURRENT_BINARY_DIR}
  )

#
# .deb
# 
set(DEB_SRC_DIR "${CMAKE_SOURCE_DIR}/build/deb/py")
set(DEB_BIN_DIR "${CMAKE_BINARY_DIR}/deb/py/debian")

configure_file("${DEB_SRC_DIR}/prepare.in" "${CMAKE_BINARY_DIR}/deb/py/prepare" @ONLY)
configure_file("${DEB_SRC_DIR}/postinst" "${DEB_BIN_DIR}/DEBIAN/postinst" COPYONLY)
configure_file("${DEB_SRC_DIR}/prerm" "${DEB_BIN_DIR}/DEBIAN/prerm" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/build/deb/control.in" "${CMAKE_BINARY_DIR}/deb/py/control.stage" @ONLY)

add_custom_target(deb-py
  COMMAND "${CMAKE_BINARY_DIR}/deb/deb-build.sh" "py" "${PYTHON_INSTALL_DIR}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/deb/")

add_dependencies(deb-py dist-py)
add_dependencies(deb-all deb-py)

