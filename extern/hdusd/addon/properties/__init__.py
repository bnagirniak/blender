# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


class HdUSDProperties(bpy.types.PropertyGroup):
    bl_type = None

    @classmethod
    def register(cls):
        cls.bl_type.hdusd = bpy.props.PointerProperty(
            name="HdUSD properties",
            description="HdUSD properties",
            type=cls,
        )

    @classmethod
    def unregister(cls):
        del cls.bl_type.hdusd


from . import (
    preferences,
    scene,
    object,
    usd_stage,
    material,
    window_manager,
    hdrpr_render,
)
register, unregister = bpy.utils.register_classes_factory((
    preferences.HDUSD_MX_OP_install_delegate,
    preferences.HDUSD_ADDON_PT_preferences,

    hdrpr_render.QualitySettings,
    hdrpr_render.InteractiveQualitySettings,
    hdrpr_render.ContourSettings,
    hdrpr_render.DenoiseSettings,
    hdrpr_render.RenderSettings,

    scene.FinalRenderSettings,
    scene.ViewportRenderSettings,
    scene.SceneProperties,

    object.ObjectProperties,

    material.MaterialProperties,

    usd_stage.UsdStagePrim,
    usd_stage.UsdStage,

    window_manager.WindowManagerProperties,
))
