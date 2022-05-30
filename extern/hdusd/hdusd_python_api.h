/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* create python module _hdusd used by addon */

PyObject *HdUSD_initPython(void);
PyObject *HdUSD_usd_node_initPython(void);

#ifdef __cplusplus
}
#endif
