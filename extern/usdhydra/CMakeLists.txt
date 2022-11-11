# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

if(WIN32)
  add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

# USD headers use deprecated TBB headers, silence warning.
add_definitions(-DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DBOOST_DEBUG_PYTHON")

set(INC
  ../gflags/src
  # ../../intern
  ../../intern/glew-mx
  ../../intern/guardedalloc
  # ../../mikktspace
  ../../source/blender/makesdna
  ../../source/blender/makesrna
  ../../source/blender/blenlib
  ../../source/blender/depsgraph
  ../../source/blender/blenkernel
  ../../source/blender/io/usd
  ../../source/blender/io/common
  ../../source/blender/io/usd/intern
  ${CMAKE_BINARY_DIR}/source/blender/makesrna/intern
)

set(INC_SYS
  ${PYTHON_INCLUDE_DIRS}
  ${USD_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIR}
  ${TBB_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIRS}
  ${GFLAGS_INCLUDE_DIRS}
)

set(SRC
  python.cpp

  session.h
  session.cpp

  utils.h
  utils.cpp

  stage.h
  stage.cpp

  camera.h
  camera.cpp

  view_settings.h
  view_settings.cpp

  usdImagingLite/engine.cpp
  usdImagingLite/engine.h
  usdImagingLite/renderDataDelegate.cpp
  usdImagingLite/renderDataDelegate.h
  usdImagingLite/renderTask.cpp
  usdImagingLite/renderTask.h
  usdImagingLite/renderParams.h
)

set(LIB
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${BOOST_PYTHON_LIBRARIES}
)

set(ADDON_FILES
  addon/__init__.py
  addon/engine.py
  addon/logger.py
  addon/preferences.py
  addon/properties.py
  addon/ui.py
)

set(ADDON_STORM_FILES
  addon/storm/__init__.py
  addon/storm/engine.py
  addon/storm/properties.py
  addon/storm/ui.py
)

set(USDHYDRA_INSTALL_PATH "scripts/addons/usdhydra")

add_definitions(${GFLAGS_DEFINES})
add_definitions(${GLOG_DEFINES})
add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
add_definitions(-DBLENDER_GFLAGS_NAMESPACE=${GFLAGS_NAMESPACE})


blender_add_lib(extern_usdhydra "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(extern_usdhydra bf_rna)

delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_FILES}" ${USDHYDRA_INSTALL_PATH})
delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_STORM_FILES}" ${USDHYDRA_INSTALL_PATH}/storm)
