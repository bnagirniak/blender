# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

if(WIN32)
  add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
add_definitions(-DPXR_STATIC)

# USD headers use deprecated TBB headers, silence warning.
add_definitions(-DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)

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
  ${USD_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIR}
  ${TBB_INCLUDE_DIR}
)

set(SRC
  python.cpp
  session.cpp

  hdusd_python_api.h
  session.h
)

set(LIB
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
  ${BOOST_LIBRARIES}
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
#  addon/operators.py
#  addon/properties.py
#  addon/ui.py
)
set(HDUSD_INSTALL_PATH "scripts/addons/hdusd")


blender_add_lib(extern_hdusd "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

#message(FATAL_ERROR "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}, CYCLES_INSTALL_PATH=${HDUSD_INSTALL_PATH}")
delayed_install(${CMAKE_CURRENT_SOURCE_DIR} "${ADDON_FILES}" ${HDUSD_INSTALL_PATH})
