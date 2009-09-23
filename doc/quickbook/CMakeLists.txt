# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#

# Targets:
#  doc-from-sources  ... generates reference
#  build-html        ... builds main.qbk
#  doc               ... build-html + doc-from-sources

find_program(XSLTPROC_EXECUTABLE xsltproc)

if (DOCUMENTATION_ONLY)
  find_package(Doxygen)

  message(STATUS "\n-----[ Doc Configuration Summary ]---------------------\n")
  message(STATUS "xsltproc: ${XSLTPROC_EXECUTABLE}")
  message(STATUS "gccxml: ${GCCXML}")
  message(STATUS "doxygen: ${DOXYGEN_EXECUTABLE}")
  message(STATUS "")
  if(NOT XSLTPROC_EXECUTABLE OR NOT DOXYGEN_EXECUTABLE OR NOT GCCXML)
    message(FATAL_ERROR "One or more tools cannot be found.")
  endif()
endif()


# doxygen xml and doc generated from sources
set(GENERATED_DOC_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated-doc")

add_custom_target(doc)

# ---------------------------------------------------------------------------
#     doc-from-sources .. builds doxygen xml files and generates docs from
#                         sources - destination is ${GENERATED_DOC_DIR}
#     

if (DOXYGEN_EXECUTABLE AND GCCXML)
  file(MAKE_DIRECTORY ${GENERATED_DOC_DIR})

  get_gccxml_flags(GCCXML_FLAGS)

  FIND_PACKAGE(Boost ${REQ_BOOST_VERSION})

  # add boost include directories as some of the processed files depend on it
  if (Boost_FOUND)
    list(APPEND GCCXML_FLAGS "-I${Boost_INCLUDE_DIRS}")
  endif()

  add_custom_target(doc-from-sources
    COMMAND
    # use system default python instead of ${PYTHON_EXECUTABLE} because of PyXML
    python "${CMAKE_SOURCE_DIR}/code/tools/source_parsing/generator.py"
    ${GCCXML_FLAGS}
    -d
    --doc-dir="${GENERATED_DOC_DIR}"
    "${CMAKE_SOURCE_DIR}/code/tools/source_parsing/jagbasecfg.py" 
    jagpdf_config 
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
  set(JAG_REFERENCE_QBK "include generated_doc.qbk")
  set(JAG_REFERENCE_LINKS_QBK "include generated_ref_links.qbk")
else()
  # the problem with this branch is that if there is not
  # generated_ref_links.qbk, then quickbook reports an error when enounters a
  # template defined in that file
  add_custom_target(doc-from-sources)
  set(JAG_REFERENCE_QBK "/ cmake: no reference generated")
  set(JAG_REFERENCE_LINKS_QBK "/ cmake: no reference generated")
endif()

add_dependencies(doc doc-from-sources)


# ---------------------------------------------------------------------------
#
set(XSLT_PARAMS)
macro(add_xslt_param PARAM VAL)
  list(APPEND XSLT_PARAMS "--stringparam")
  list(APPEND XSLT_PARAMS "${PARAM}")
  list(APPEND XSLT_PARAMS "\"${VAL}\"")
endmacro(add_xslt_param)

add_xslt_param("admon.graphics.path" "images/") 
add_xslt_param("admon.graphics" "1") 
add_xslt_param("admon.style" "") 
add_xslt_param("admon.textlabel" "0") 
add_xslt_param("base.dir" "doc/") 
add_xslt_param("boost.image.alt" "JagPDF") 
add_xslt_param("boost.image.h" "50") 
add_xslt_param("boost.image.src" "images/logo.png") 
add_xslt_param("boost.image.w" "111") 
add_xslt_param("boostbook.main.css" "doc.css") 
add_xslt_param("chunk.first.sections" "1") 
add_xslt_param("chunk.section.depth" "2") 
add_xslt_param("chunker.output.encoding" "utf-8") 
add_xslt_param("css.stylesheet.root" "")
add_xslt_param("generate.id.attributes" "1")
add_xslt_param("generate.manifest" "1")
add_xslt_param("generate.section.toc.level" "3")
# use ';' instead of ' ' to make it a cmake list, when passed to a COMMAND it is
# replaced with ' '
add_xslt_param("generate.toc" "section;toc;article;toc;chapter;toc;book;toc")
add_xslt_param("html.ext" ".htm")
add_xslt_param("html.stylesheet" "doc.css")
add_xslt_param("nav.layout" "none")
add_xslt_param("navig.graphics.extension" ".png")
add_xslt_param("navig.graphics.path" "images/")
add_xslt_param("navig.graphics" "1")
add_xslt_param("root.filename" "index")
add_xslt_param("section.autolabel.max.depth" "2")
add_xslt_param("section.autolabel" "1")
add_xslt_param("toc.section.depth" "3")


#message(FATAL_ERROR ${XSLT_PARAMS})
  
set(BOOSTBOOK_CATALOG "${CMAKE_CURRENT_BINARY_DIR}/boostbook_catalog.xml")
configure_file(
  "${CMAKE_SOURCE_DIR}/doc/quickbook/boostbook_catalog.xml.in"
  ${BOOSTBOOK_CATALOG}
  @ONLY)


# quickbook src -> quickbook xml
set(QUICKBOOK_XML "${CMAKE_CURRENT_BINARY_DIR}/main.xml")
set(QUICKBOOK_SRC_IN "${CMAKE_CURRENT_SOURCE_DIR}/main.qbk.in")
set(QUICKBOOK_SRC "${CMAKE_CURRENT_BINARY_DIR}/main.qbk")

configure_file("${QUICKBOOK_SRC_IN}" "${QUICKBOOK_SRC}" @ONLY)

add_custom_command(
  OUTPUT "${QUICKBOOK_XML}"
  COMMAND quickbook  -I${CMAKE_CURRENT_SOURCE_DIR} -I${GENERATED_DOC_DIR} --output-file=${QUICKBOOK_XML} ${QUICKBOOK_SRC}
  DEPENDS ${QUICKBOOK_SRC}
)

# quickbook xml -> docbook xml
set(DOCBOOK_XML "${CMAKE_CURRENT_BINARY_DIR}/main.docbook")
add_custom_command(
  OUTPUT "${DOCBOOK_XML}"
  COMMAND 
  env XML_CATALOG_FILES=${BOOSTBOOK_CATALOG} ${XSLTPROC_EXECUTABLE} ${XSLT_PARAMS}
  --xinclude 
  -o ${DOCBOOK_XML} ${CMAKE_SOURCE_DIR}/external/doc/boostbook/xsl/docbook.xsl
  ${QUICKBOOK_XML}
  DEPENDS ${QUICKBOOK_XML}
)

add_custom_target(doc-html
  COMMAND
  env XML_CATALOG_FILES=${BOOSTBOOK_CATALOG} ${XSLTPROC_EXECUTABLE} ${XSLT_PARAMS}
  --stringparam manifest "pure-doc_HTML.manifest" 
  --xinclude -o "html/"
  ${CMAKE_SOURCE_DIR}/external/doc/boostbook/xsl/html.xsl ${DOCBOOK_XML}
  # postprocess generated html
  COMMAND
  ${CMAKE_SOURCE_DIR}/build/scripts/add_section_ids.sh "html/pure-doc_HTML.manifest"
  COMMAND rm "html/pure-doc_HTML.manifest"
  DEPENDS ${DOCBOOK_XML}
)

# ---------------------------------------------------------------------------
#   installation

# Doc
set(JAGPDF_DOC_INSTALL_PREFIX "doc")


install( # generated
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
  DESTINATION ${JAGPDF_DOC_INSTALL_PREFIX}
  COMPONENT jagpdf-doc)

install( # .
  FILES
  doc.css
  ${CMAKE_SOURCE_DIR}/../jagpdf-www/base.css
  DESTINATION ${JAGPDF_DOC_INSTALL_PREFIX}/doc
  COMPONENT jagpdf-doc)

install( # images
  DIRECTORY images/
  DESTINATION ${JAGPDF_DOC_INSTALL_PREFIX}/doc/images
  COMPONENT jagpdf-doc
  FILES_MATCHING PATTERN "*.png"
  PATTERN ".svn" EXCLUDE)

install( # ..
  FILES
  docindex.htm
  DESTINATION ${JAGPDF_DOC_INSTALL_PREFIX}
  COMPONENT jagpdf-doc)

install( # samples
  DIRECTORY ${CMAKE_SOURCE_DIR}/code/test/apitest/reference-pdf/
  DESTINATION ${JAGPDF_DOC_INSTALL_PREFIX}/doc/samples
  COMPONENT jagpdf-doc
  FILES_MATCHING PATTERN "jagpdf_doc_*.pdf"
  PATTERN ".svn"  EXCLUDE)



add_custom_target(dist-doc
  COMMAND ${CMAKE_COMMAND}
  -D "CMAKE_INSTALL_COMPONENT=jagpdf-doc"
  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
)

add_dependencies(doc-html doc-from-sources)
add_dependencies(doc doc-html)
add_dependencies(dist-doc doc)

add_custom_target(pack-doc
  COMMAND tar -cjvf jagpdf-${JAGPDF_VERSION}.doc.tar.bz2 
  --directory=${CMAKE_INSTALL_PREFIX} 
  --transform='s,^doc,jagpdf-${JAGPDF_VERSION}/doc,1' 
  doc
)

#
# bjam -n -a /doc//doc .. to see commands
#



# --xinclude 
# -o "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.docbook" 
# "/home/jarda/code/jagbase/build/boostbook/xsl/docbook.xsl" 
# "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.xml"
# 
# 
# "xsltproc" 
# --stringparam admon.graphics.path "images/" 
# --stringparam admon.graphics "1" 
# --stringparam admon.style "" 
# --stringparam admon.textlabel "0" 
# --stringparam base.dir "doc/" 
# --stringparam boost.image.alt "JagPDF" 
# --stringparam boost.image.h "34" 
# --stringparam boost.image.src "images/logo.png" 
# --stringparam boost.image.w "110" 
# --stringparam boostbook.main.css "doc.css" 
# --stringparam chunk.first.sections "1" 
# --stringparam chunk.section.depth "2" 
# --stringparam chunker.output.encoding "utf-8" 
# --stringparam css.stylesheet.root "" 
# --stringparam generate.id.attributes "1" 
# --stringparam generate.manifest "1" 
# --stringparam generate.section.toc.level "3" 
# --stringparam generate.toc "section toc article toc chapter toc book toc" 
# --stringparam html.ext ".htm" 
# --stringparam html.stylesheet "doc.css" 
# --stringparam manifest "pure-doc_HTML.manifest" 
# --stringparam nav.layout "none" 
# --stringparam navig.graphics.extension ".png" 
# --stringparam navig.graphics.path "images/" 
# --stringparam navig.graphics "1" 
# --stringparam root.filename "index" 
# --stringparam section.autolabel.max.depth "2" 
# --stringparam section.autolabel "1" 
# --stringparam toc.section.depth "3" 
# --xinclude 
# -o "html/" 
# "/home/jarda/code/jagbase/build/boostbook/xsl/html.xsl" 
# "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.docbook"

# "xsltproc" --stringparam admon.graphics.path "images/" --stringparam admon.graphics "1" --stringparam admon.style "" --stringparam admon.textlabel "0" --stringparam base.dir "doc/" --stringparam boost.image.alt "JagPDF" --stringparam boost.image.h "34" --stringparam boost.image.src "images/logo.png" --stringparam boost.image.w "110" --stringparam boostbook.main.css "doc.css" --stringparam chunk.first.sections "1" --stringparam chunk.section.depth "2" --stringparam chunker.output.encoding "utf-8" --stringparam css.stylesheet.root "" --stringparam generate.id.attributes "1" --stringparam generate.manifest "1" --stringparam generate.section.toc.level "3" --stringparam generate.toc "section toc article toc chapter toc book toc" --stringparam html.ext ".htm" --stringparam html.stylesheet "doc.css" --stringparam nav.layout "none" --stringparam navig.graphics.extension ".png" --stringparam navig.graphics.path "images/" --stringparam navig.graphics "1" --stringparam root.filename "index" --stringparam section.autolabel.max.depth "2" --stringparam section.autolabel "1" --stringparam toc.section.depth "3" --xinclude -o "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.docbook" "/home/jarda/code/jagbase/build/boostbook/xsl/docbook.xsl" "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.xml"
# "xsltproc" --stringparam admon.graphics.path "images/" --stringparam admon.graphics "1" --stringparam admon.style "" --stringparam admon.textlabel "0" --stringparam base.dir "doc/" --stringparam boost.image.alt "JagPDF" --stringparam boost.image.h "34" --stringparam boost.image.src "images/logo.png" --stringparam boost.image.w "110" --stringparam boostbook.main.css "doc.css" --stringparam chunk.first.sections "1" --stringparam chunk.section.depth "2" --stringparam chunker.output.encoding "utf-8" --stringparam css.stylesheet.root "" --stringparam generate.id.attributes "1" --stringparam generate.manifest "1" --stringparam generate.section.toc.level "3" --stringparam generate.toc "section toc article toc chapter toc book toc" --stringparam html.ext ".htm" --stringparam html.stylesheet "doc.css" --stringparam nav.layout "none" --stringparam navig.graphics.extension ".png" --stringparam navig.graphics.path "images/" --stringparam navig.graphics "1" --stringparam root.filename "index" --stringparam section.autolabel.max.depth "2" --stringparam section.autolabel "1" --stringparam toc.section.depth "3" --xinclude -o "html/" "/home/jarda/code/jagbase/build/boostbook/xsl/html.xsl" "/home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/main.docbook"

#   bash /home/jarda/code/jagbase/build/scripts/add_section_ids.sh /home/jarda/code/jagbase/stage/doc/quickbook/gcc-4.2.4/debug/doc html/pure-doc_HTML.manifest && rm "html/pure-doc_HTML.manifest"