# Copyright 2015 The Edda Authors. All rights reserved.
# Use of this source code is governed by a MIT-style license that can be
# found in the LICENSE file.

set(sources
  uncertainIsocontourVTK.cpp
  streamlineMonteCarlo.cpp
  streamlineGaussianVTK.cpp
  randomSample.cpp
  denseSamplingField.cpp
  fuzzyIsocontourVTK.cpp
  test_vtk_filters.cpp
  gmm_raw2vtk.cpp
)

IF(APPLE)
  # The primitive VTK renderer cannot catch keyboard events
  # without packed into the mac bundle
  SET(EXECUTABLE_FLAG MACOSX_BUNDLE)
ENDIF(APPLE)

foreach (file ${sources})
  get_filename_component(name ${file} NAME_WE)
  if (EDDA_BUILD_WITH_CUDA)
    cuda_add_executable ( ${name} ${EXECUTABLE_FLAG} ${file} )
  else ()
    SET_SOURCE_FILES_PROPERTIES(${file} PROPERTIES LANGUAGE CXX)
    add_executable ( ${name} ${EXECUTABLE_FLAG} ${file} )
  endif ()

  target_link_libraries ( ${name} edda edda_vis ${LINK_LIBS} )
endforeach ()

