# Copyright 2015 The Edda Authors. All rights reserved.
# Use of this source code is governed by a MIT-style license that can be
# found in the LICENSE file.

set (SRC_DIRS filters io)

if (EDDA_BUILD_VTK_FILTERS)
  list(APPEND SRC_DIRS vtk)
endif ()

foreach (dir in ${SRC_DIRS})
  file(GLOB tmp ${dir}/*.cpp ${dir}/*.cu)
  list(APPEND SRC_FILES ${tmp})
  file(GLOB tmp  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.h)
  list(APPEND HEADER_FILES ${tmp})
endforeach ()

#file (GLOB_RECURSE SRC_FILES "*.cpp")
#file (GLOB_RECURSE HEADER_FILES "*.h")
#message(warning "${SRC_FILES} ")

if (EDDA_BUILD_WITH_CUDA)
  cuda_add_library (${PROJECT_NAME} STATIC ${SRC_FILES} ${HEADER_FILES})
else ()
  add_library (${PROJECT_NAME} ${LIB_TYPE} ${SRC_FILES} ${HEADER_FILES})
endif ()

### for building shared library (BUILD_SHARED_LIBS=ON), otherwise this does nothing:
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

set_target_properties (${PROJECT_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${PROJECT_NAME}_d"
    RELEASE_OUTPUT_NAME "${PROJECT_NAME}")

#install (
#    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
#    DESTINATION include/${PROJECT_NAME}
#    FILES_MATCHING PATTERN "*.h")

foreach(HEADER ${HEADER_FILES})
  string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
  install(FILES ${HEADER} DESTINATION include/${PROJECT_NAME}/${DIR})
endforeach()

install (
    TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME})


if (EDDA_BUILD_PARAVIEW_PLUGINS)
  add_subdirectory(vtk)
endif ()


