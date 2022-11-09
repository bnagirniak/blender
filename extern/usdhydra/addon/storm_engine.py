# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

from bpy.utils import register_class, unregister_class

from .engine import USDHydraEngine
from .preferences import addon_preferences


class USDHydraHdStormEngine(USDHydraEngine):
    bl_idname = 'USDHydraHdStormEngine'
    bl_label = "USD Hydra: Storm"
    bl_info = "USD Hydra Storm (OpenGL) render delegate"

    bl_use_preview = False
    bl_use_gpu_context = True

    delegate_name = "HdStormRendererPlugin"


def register():
    if addon_preferences().storm_delegate:
        register_class(USDHydraHdStormEngine)


def unregister():
    if addon_preferences().storm_delegate:
        unregister_class(USDHydraHdStormEngine)
