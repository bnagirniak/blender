# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

if(WIN32)
  add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN -DBOOST_DEBUG_PYTHON)
endif()

# USD headers use deprecated TBB headers, silence warning.
add_definitions(-DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)

add_definitions(${GFLAGS_DEFINES})
add_definitions(${GLOG_DEFINES})
add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
add_definitions(-DBLENDER_GFLAGS_NAMESPACE=${GFLAGS_NAMESPACE})

add_definitions(-DBOOST_ALL_NO_LIB)

set(INC
  ../../../../intern/glew-mx
  ../../../../intern/guardedalloc
  ../../makesdna
  ../../makesrna
  ../../blenlib
  ../../depsgraph
  ../../blenkernel
  ../../io/usd
  ../../io/common
  ../../io/usd/intern
  ../../gpu
  ../../gpu/opengl
  ../../gpu/intern
  ${CMAKE_BINARY_DIR}/source/blender/makesrna/intern
)

set(INC_SYS
  ${Epoxy_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${USD_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIR}
  ${TBB_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIRS}
  ${GFLAGS_INCLUDE_DIRS}
)

set(SRC
  python.cpp

  engine.h
  engine.cpp
  finalEngine.h
  finalEngine.cpp
  finalEngineGL.h
  finalEngineGL.cpp
  viewportEngine.h
  viewportEngine.cpp

  utils.h
  utils.cpp

  renderTaskDelegate.cpp
  renderTaskDelegate.h

  sceneDelegate/blenderSceneDelegate.h
  sceneDelegate/blenderSceneDelegate.cpp
  sceneDelegate/object.h
  sceneDelegate/object.cpp
  sceneDelegate/mesh.h
  sceneDelegate/mesh.cpp
  sceneDelegate/light.h
  sceneDelegate/light.cpp
  sceneDelegate/scene.h
  sceneDelegate/scene.cpp
  sceneDelegate/material.h
  sceneDelegate/material.cpp
)

set(LIB
  ${Epoxy_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${BOOST_LIBRARIES}
)

set(ADDON_FILES
  addon/__init__.py
  addon/engine.py
  addon/logger.py
  addon/preferences.py
  addon/ui.py
  addon/matx.py
)

set(ADDON_STORM_FILES
  addon/storm/__init__.py
  addon/storm/engine.py
  addon/storm/properties.py
  addon/storm/ui.py
)

blender_add_lib(bf_render_hydra "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

#add_dependencies(extern_usdhydra bf_rna)

#set(USDHYDRA_INSTALL_PATH "scripts/addons/usdhydra")
#delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_FILES}" ${USDHYDRA_INSTALL_PATH})
#delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_STORM_FILES}" ${USDHYDRA_INSTALL_PATH}/storm)
