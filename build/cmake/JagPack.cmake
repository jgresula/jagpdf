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
    SET(CPACK_GENERATOR "TBZ2;DEB")
    SET(CPACK_SOURCE_GENERATOR "TBZ2")
    execute_process(
      COMMAND "dpkg" "--print-architecture"
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      ERROR_QUIET)
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
  MACRO(ADD_COMPONENT_PACKAGE 
      __component
      # ARGV1 - optionally package file name stem, otherwise __component
      )
    SET(PACKAGE_COMPONENT ${__component})
    IF("${ARGV1}" STREQUAL "")
      SET(PACKAGE_COMPONENT_NAME ${PACKAGE_COMPONENT})
    ELSE()
      SET(PACKAGE_COMPONENT_NAME ${ARGV1})
    ENDIF()

    # get platform
    string(REGEX MATCH "x86|i.86" MATCH "${CMAKE_SYSTEM_PROCESSOR}")
    if (MATCH)
      set(JAG_SYSTEM_PROCESSOR "x86")
    else()
      set(JAG_SYSTEM_PROCESSOR "CMAKE_SYSTEM_PROCESSOR")
    endif()
    if(WIN32 AND NOT CYGWIN)
      set(JAG_SYSTEM_NAME "win32")
    else()
      set(JAG_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    endif()
    SET(JAGPDF_PACK_PLATFORM "${JAG_SYSTEM_NAME}.${JAG_SYSTEM_PROCESSOR}")
    STRING(TOLOWER ${JAGPDF_PACK_PLATFORM} JAGPDF_PACK_PLATFORM)
    # special case, if we are packing documentation, do not include platform
    IF("${__component}" MATCHES "jagpdf-doc")
      SET(JAGPDF_PACK_PLATFORM)
    ENDIF()
    
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