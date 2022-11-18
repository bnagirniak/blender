# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from ..engine import HydraRenderEngine


class HdStormHydraRenderEngine(HydraRenderEngine):
    bl_idname = 'HdStormHydraRenderEngine'
    bl_label = "Hydra: Storm"
    bl_info = "Hydra Storm (OpenGL) render delegate"

    bl_use_preview = False
    bl_use_gpu_context = True

    delegate_id = 'HdStormRendererPlugin'

    def get_delegate_settings(self, engine_type):
        settings = bpy.context.scene.usdhydra_storm
        return {
            'enableTinyPrimCulling': settings.enable_tiny_prim_culling,
            'maxLights': settings.max_lights,
        }
