# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from ..engine import HydraRenderEngine
from ..preferences import addon_preferences


class HdStormHydraRenderEngine(HydraRenderEngine):
    bl_idname = 'HdStormHydraRenderEngine'
    bl_label = "Hydra: Storm"
    bl_info = "Hydra Storm (OpenGL) render delegate"

    bl_use_preview = False
    bl_use_gpu_context = True

    delegate_name = "HdStormRendererPlugin"


def register():
    if addon_preferences().storm_render_engine:
        bpy.utils.register_class(HdStormHydraRenderEngine)


def unregister():
    if addon_preferences().storm_render_engine:
        bpy.utils.unregister_class(HdStormHydraRenderEngine)
