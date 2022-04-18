/* SPDX-License-Identifier: Apache-2.0
 * Copyright 2011-2022 Blender Foundation */

#ifndef __HDUSD_PYTHON_API_H__
#define __HDUSD_PYTHON_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/* create python module _hdusd used by addon */

PyObject *HdUSD_initPython(void);

#ifdef __cplusplus
}
#endif

#endif /* __HDUSD_PYTHON_API_H__ */
