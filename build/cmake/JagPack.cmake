# Copyright (c) 2005-2009 Jaroslav Gresula
# 
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


GET_FILENAME_COMPONENT(__cmake_path ${CMAKE_COMMAND} PATH)
FIND_PROGRAM(CPACK_COMMAND cpack ${__cmake_path})
MESSAGE(STATUS "Found CPack at: ${CPACK_COMMAND}")
IF(NOT CPACK_COMMAND)
    MESSAGE(FATAL_ERROR "Need CPack!")
ENDIF(NOT CPACK_COMMAND)

ADD_CUSTOM_TARGET(ALL_PACKAGES)
SET(SRC_IN ${CMAKE_SOURCE_DIR}/build/cmake/cpack)

if(CPACK_COMMAND)
  IF(UNIX)
    SET(CPACK_GENERATOR "TBZ2")
    SET(CPACK_SOURCE_GENERATOR "TBZ2")
  ELSEIF(WIN32)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_SOURCE_GENERATOR "ZIP")
  ENDIF()

  set(SRC_STEM "jagpdf-${JAGPDF_VERSION}")

  #
  # source code
  #
  CONFIGURE_FILE(${SRC_IN}/CPackSourceConfig.in CPackSourceConfig @ONLY)
  ADD_CUSTOM_TARGET(PACKAGE_source
    ${CPACK_COMMAND} --config CPackSourceConfig CPackSourceConfig)
  ADD_DEPENDENCIES(ALL_PACKAGES PACKAGE_source)

  #
  # all source code
  #
  CONFIGURE_FILE(${SRC_IN}/CPackSourceAllConfig.in CPackSourceAllConfig @ONLY)
  ADD_CUSTOM_TARGET(PACKAGE_source_all
    ${CPACK_COMMAND} --config CPackSourceAllConfig CPackSourceAllConfig)
  ADD_DEPENDENCIES(ALL_PACKAGES PACKAGE_source_all)


  #
  # apitests code
  #
  CONFIGURE_FILE(${SRC_IN}/CPackSourceApitestsConfig.in CPackSourceApitestsConfig @ONLY)
  ADD_CUSTOM_TARGET(PACKAGE_apitests
    ${CPACK_COMMAND} --config CPackSourceApitestsConfig CPackSourceApitestsConfig)
  ADD_DEPENDENCIES(ALL_PACKAGES PACKAGE_apitests)

  #
  # binaries
  #
  MACRO(ADD_COMPONENT_PACKAGE _component _stem _debname)
    SET(PACKAGE_COMPONENT ${_component})
    SET(PACKAGE_COMPONENT_NAME ${_stem})
    SET(CPACK_PACKAGE_NAME ${_debname})

    SET(__packageTarget PACKAGE_${PACKAGE_COMPONENT})
    SET(__packageConfig ${CMAKE_CURRENT_BINARY_DIR}/CPackConfig-${PACKAGE_COMPONENT}.cmake)
    
    CONFIGURE_FILE(${SRC_IN}/CPackConfig.in ${__packageConfig} @ONLY)
    ADD_CUSTOM_TARGET(${__packageTarget}
      ${CPACK_COMMAND} --config "${__packageConfig}")
    ADD_DEPENDENCIES(ALL_PACKAGES ${__packageTarget})
  ENDMACRO(ADD_COMPONENT_PACKAGE) 
else()
  MACRO(ADD_COMPONENT_PACKAGE _ignore)
  # cpack not found, just empty macros
endif()