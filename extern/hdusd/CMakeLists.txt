# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

set(INC
  # ../../intern
  # ../../glew-mx
  ../../intern/guardedalloc
  # ../../mikktspace
  ../../source/blender/makesdna
  ../../source/blender/makesrna
  ../../source/blender/blenlib
  ${CMAKE_BINARY_DIR}/source/blender/makesrna/intern
)

set(INC_SYS
  ${PYTHON_INCLUDE_DIRS}
)

set(SRC
  python.cpp
  session.cpp
  usd_node.cpp

  hdusd_python_api.h
  session.h
)

set(LIB

  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
)

#if(WITH_CYCLES_LOGGING)
  list(APPEND LIB
    ${GLOG_LIBRARIES}
    ${GFLAGS_LIBRARIES}
  )
#endif()

set(ADDON_FILES
  addon/__init__.py
  addon/engine.py
)

set(ADDON_USD_NODES_FILES
  addon/usd_nodes/__init__.py
  addon/usd_nodes/node_tree.py
)

set(ADDON_USD_NODES_NODES_FILES
  addon/usd_nodes/nodes/__init__.py
  addon/usd_nodes/nodes/base_node.py
  addon/usd_nodes/nodes/converter.py
  addon/usd_nodes/nodes/input.py
  addon/usd_nodes/nodes/output.py
  addon/usd_nodes/nodes/transformations.py
)

set(HDUSD_INSTALL_PATH "scripts/addons/hdusd")


blender_add_lib(extern_hdusd "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

#message(FATAL_ERROR "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}, CYCLES_INSTALL_PATH=${HDUSD_INSTALL_PATH}")
delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_FILES}" ${HDUSD_INSTALL_PATH})
delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_USD_NODES_FILES}" ${HDUSD_INSTALL_PATH}/usd_nodes)
delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_USD_NODES_NODES_FILES}" ${HDUSD_INSTALL_PATH}/usd_nodes/nodes)
