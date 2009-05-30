# Copyright (c) 2005-2009 Jaroslav Gresula
#
# Distributed under the MIT license (See accompanying file
# LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
#


include_directories(${CMAKE_BINARY_DIR}/include/messages)
include_directories(${CMAKE_BINARY_DIR}/include)

set(CXX_TEST_PATH ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DJAGAPI_BUILDING_CPP)


macro(add_messages_dependency TARGET)
  add_dependencies(${TARGET} msg_errlib_TARGET msg_jstd_TARGET msg_resources_TARGET)
endmacro(add_messages_dependency)

#
# -- unit tests driven by the driver
#
create_test_sourcelist(Tests unittestdriver.cpp
  unicodeto8bit.cpp
  exceptions.cpp
  config.cpp
  optsparser_test.cpp
  mmapfile.cpp
  errortls.cpp
)

add_executable(unittestdriver ${Tests})
target_link_libraries(unittestdriver pdflib-static-core)
add_messages_dependency(unittestdriver)

set(TestsToRun ${Tests})
remove(TestsToRun unittestdriver.cpp)

foreach(test ${TestsToRun})
  get_filename_component(TName ${test} NAME_WE)
  add_test(ut_${TName} ${CXX_TEST_PATH}/unittestdriver ${TName})
endforeach()



#
# -- a special test that dynamically loads a shared library; it is not driven by
# -- the driver
#
add_library(dllmytss SHARED tssdll.cpp)
target_link_libraries(dllmytss pdflib-static-core)
add_messages_dependency(dllmytss)

add_executable(ut_tss tss.cpp)
target_link_libraries(ut_tss pdflib-static-core)
add_messages_dependency(ut_tss)

add_test(ut_tss ${CXX_TEST_PATH}/ut_tss)

if (JAG_GCC_IN_UNIX)
  set_target_properties(dllmytss PROPERTIES LINK_FLAGS "-ldl")
  set_target_properties(ut_tss PROPERTIES LINK_FLAGS "-ldl -Wl,-rpath=${CXX_TEST_PATH}")
endif()




#
# -- main unit-tests target
#
add_custom_target(unit-tests
  DEPENDS unittestdriver ut_tss dllmytss
  COMMAND ${CMAKE_CTEST_COMMAND} -R ut_)

