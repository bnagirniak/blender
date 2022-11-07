# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

from .engine import USDHydraEngine


class USDHydraHdStormEngine(USDHydraEngine):
    bl_idname = 'USDHydraHdStormEngine'
    bl_label = "USD Hydra: Storm"
    bl_info = "USD Hydra Storm (OpenGL) render delegate"

    bl_use_preview = False
    bl_use_gpu_context = True

    delegate_name = "HdStormRendererPlugin"
