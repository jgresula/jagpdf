# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

# generate messages and rc
jag_process_message_file(msg_pdflib)
jag_create_rc_file(jagpdf "JagPDF library for C/C++.")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# signal that pdflib is being built, not used
add_definitions(-DJAGAPI_BUILDING_CPP)

# generate api files
set(API_IMPL_DIR ${CMAKE_BINARY_DIR})
file(MAKE_DIRECTORY ${API_HEADER_DIR})
file(MAKE_DIRECTORY ${API_SWIG_DIR})

set(PREBUILT_API_DIR "${CMAKE_SOURCE_DIR}/code/src/generated/api")
if(GCCXML)
  # generates the api files for C, C++ and SWIG
  get_gccxml_flags(GCCXML_FLAGS)
  add_custom_command(
    OUTPUT
    "${API_HEADER_DIR}/capi.h"
    "${API_HEADER_DIR}/cppapi.h"
    "${API_IMPL_DIR}/capiimpl.cpp"
    "${API_SWIG_DIR}/generated.swg"
    DEPENDS #TBD
    ${CMAKE_SOURCE_DIR}/code/include/interfaces/configuration.h
    ${CMAKE_SOURCE_DIR}/code/include/pdflib/apifreefuns.h
    ${CMAKE_SOURCE_DIR}/code/include/pdflib/interfaces/pdfpage.h
    ${CMAKE_SOURCE_DIR}/code/include/pdflib/interfaces/pdfwriter.h
    ${CMAKE_SOURCE_DIR}/code/include/pdflib/interfaces/canvas.h
    ${CMAKE_SOURCE_DIR}/code/include/pdflib/interfaces/docoutline.h
    ${CMAKE_SOURCE_DIR}/code/include/resources/interfaces/fontinfo.h
    ${CMAKE_SOURCE_DIR}/code/include/resources/interfaces/imagespec.h
    ${CMAKE_SOURCE_DIR}/code/include/resources/interfaces/imageproperties.h
    ${CMAKE_SOURCE_DIR}/code/include/resources/interfaces/imagemask.h
    COMMAND
    ${PYTHON_EXECUTABLE} ${JAG_TOOLS_DIR}/source_parsing/generator.py --c-api --cpp-api --swig-file ${GCCXML_FLAGS} --api-header-dir=${API_HEADER_DIR} --api-impl-dir=${API_IMPL_DIR} --api-swig-dir=${API_SWIG_DIR} ${JAG_TOOLS_DIR}/source_parsing/jagbasecfg.py jagpdf_config
    # copy the generated api files to the source tree if they changed
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${API_HEADER_DIR}/capi.h" "${PREBUILT_API_DIR}/capi.h.in"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${API_HEADER_DIR}/cppapi.h" "${PREBUILT_API_DIR}/cppapi.h.in"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${API_IMPL_DIR}/capiimpl.cpp" "${PREBUILT_API_DIR}/capiimpl.cpp.in"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${API_SWIG_DIR}/generated.swg" "${PREBUILT_API_DIR}/generated.swg.in"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

  # a named target for the generated api files
  add_custom_target(
    generated_api_files
    DEPENDS 
    "${API_HEADER_DIR}/capi.h"
    "${API_HEADER_DIR}/cppapi.h"
    "${API_IMPL_DIR}/capiimpl.cpp"
    "${API_SWIG_DIR}/generated.swg"
    )
else()
  # gccxml not found, use the prebuilt api files
  configure_file("${PREBUILT_API_DIR}/capi.h.in" "${API_HEADER_DIR}/capi.h" COPYONLY)
  configure_file("${PREBUILT_API_DIR}/cppapi.h.in" "${API_HEADER_DIR}/cppapi.h" COPYONLY)
  configure_file("${PREBUILT_API_DIR}/capiimpl.cpp.in" "${API_IMPL_DIR}/capiimpl.cpp" COPYONLY)
  configure_file("${PREBUILT_API_DIR}/generated.swg.in" "${API_SWIG_DIR}/generated.swg" COPYONLY)
  add_custom_target(generated_api_files)
endif()

# ---------------------------------------------------------------------------
#                            SOURCES
#
set(pdflib_SOURCES
  ${API_IMPL_DIR}/capiimpl.cpp
  cfgsymbols.cpp
  capiruntime.cpp
  apifreefuns.cpp
  color.cpp
  colorspace.cpp
  resourcemanagement.cpp
  fontmanagement.cpp
  canvastext.cpp
  canvascolor.cpp
  canvasimpl.cpp
  imagexobject.cpp
  imagexobjectmask.cpp
  xobjectutils.cpp
  graphicsstate.cpp
  graphicsstatestack.cpp
  graphicsstatedictionary.cpp
  graphicsstatedictonaryobject.cpp
  fontdictionary.cpp
  fontdescriptor.cpp
  pdffontdata.cpp
  pdffont.cpp
  tounicode.cpp
  cidfontdictionary.cpp
  encdifferences.cpp
  datatypecasts.cpp
  docwriterimpl.cpp
  objfmtbasic.cpp
  objfmt.cpp
  encryption_stream.cpp
  indirectobjectimpl.cpp
  indirectobjectcallback.cpp
  indirectobjectfromdirect.cpp
  pdffile_trailer.cpp
  crossrefsection.cpp
  generic_dictionary.cpp
  generic_dictionary_impl.cpp
  catalog.cpp
  outlineitem.cpp
  docoutlineimpl.cpp
  page_object.cpp
  page_tree.cpp
  treenodeimpl.cpp
  contentstream.cpp
  resource_dictionary.cpp
  page_tree_node.cpp
  standard_security_handler.cpp
  patternimpl.cpp
  resourcelist.cpp
  defines.cpp
  annotationimpl.cpp
  destination.cpp
  ${msg_pdflib_IMPL}
)

#
# A static library with the pdf functionality.
#
# !!! Do not use this target directly unless you know what you are
# !!! doing (e.g. linking it to an executable which provides correct
# !!! initialization). Use pdflib-static instead.
#
jag_setup_precompiled_headers(pdflib_SOURCES)
add_library(
  pdflib-static-core
  STATIC
  # sources
  ${pdflib_SOURCES}
)

add_dependencies(pdflib-static-core 
  msg_errlib_TARGET 
  msg_jstd_TARGET 
  msg_resources_TARGET
  msg_pdflib_TARGET
  generated_api_files)

target_link_libraries(
  pdflib-static-core
  ${JAG_LINK_LIBS}
  errlib
  jstd
  resources
  ${JAG_THIRD_PARTY_LIBS})


# quit this file if JagPDF for C/C++ is not going to be built
if(NOT BUILD_C)
  return()
endif()


#
# Shared library for C/C++
#
add_library(jagpdf SHARED pdflibmain.cpp ${jagpdf_RC_FILE})
target_link_libraries(jagpdf pdflib-static-core)
if (JAG_GCC_IN_UNIX)
  set(JAG_PDF_LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/export.gcc.map ${LIBSTDCPP_LINK_FLAG}")
  set(JAG_PDF_OUTPUT_NAME "jagpdf")
elseif(MSVC_VERSION)
  set(JAG_PDF_LINK_FLAGS " ")
  set(JAG_PDF_OUTPUT_NAME "jagpdf-${JAGPDF_VERSION_MAJOR}.${JAGPDF_VERSION_MINOR}")
endif()

set_target_properties(jagpdf
  PROPERTIES
  SOVERSION ${JAGPDF_VERSION_MAJOR}
  VERSION ${JAGPDF_VERSION_MAJOR}.${JAGPDF_VERSION_MINOR}.${JAGPDF_VERSION_PATCH}
  LINK_FLAGS ${JAG_PDF_LINK_FLAGS}
  OUTPUT_NAME ${JAG_PDF_OUTPUT_NAME}
  )

#
# Installation definition.
#
install( # binaries
  TARGETS jagpdf
  DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT jagpdf)

install( # includes
  DIRECTORY ${CMAKE_SOURCE_DIR}/code/include/jagpdf/ ${API_HEADER_DIR}
  DESTINATION ${INCLUDE_INSTALL_DIR}/jagpdf
  COMPONENT jagpdf
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE)

if(BINARY_DISTRIBUTION)
  install( # other files
    FILES
    ${JAG_DIST_ACCOMPANYING_FILES}
    DESTINATION "."
    COMPONENT jagpdf)
endif()


#
# Installs jagpdf.
#
add_custom_target(dist-c
  COMMAND ${CMAKE_COMMAND}
  -D "CMAKE_INSTALL_COMPONENT=jagpdf"
  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
  )
add_dependencies(dist-c jagpdf)
add_dependencies(dist-all dist-c)


#
# Checks that the produced binaries are ok
#
add_custom_target(check-jagpdf-binaries
  COMMAND ${PYTHON_EXECUTABLE} ${JAG_TOOLS_DIR}/check_package.py ${CMAKE_CURRENT_BINARY_DIR}
  )
#add_dependencies(check-jagpdf-binaries dist-c)


ADD_COMPONENT_PACKAGE(jagpdf c_cpp jagpdf)

#
# .deb
# 
set(DEB_SRC_DIR "${CMAKE_SOURCE_DIR}/build/deb/c")
set(DEB_BIN_DIR "${CMAKE_BINARY_DIR}/deb/c/debian")

configure_file("${DEB_SRC_DIR}/prepare.in" "${CMAKE_BINARY_DIR}/deb/c/prepare" @ONLY)
configure_file("${DEB_SRC_DIR}/postinst" "${DEB_BIN_DIR}/DEBIAN/postinst" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/build/deb/control.in" "${CMAKE_BINARY_DIR}/deb/c/control.stage" @ONLY)

add_custom_target(deb-c
  COMMAND "${CMAKE_BINARY_DIR}/deb/deb-build.sh" "c" "${CMAKE_INSTALL_PREFIX}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/deb/")

add_dependencies(deb-c dist-c)
add_dependencies(deb-all deb-c)
