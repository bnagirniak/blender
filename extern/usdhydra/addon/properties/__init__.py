# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


class USDHydraProperties(bpy.types.PropertyGroup):
    bl_type = None

    @classmethod
    def register(cls):
        cls.bl_type.usdhydra = bpy.props.PointerProperty(
            name="USDHydra properties",
            description="USDHydra properties",
            type=cls,
        )

    @classmethod
    def unregister(cls):
        del cls.bl_type.usdhydra


from . import (
    scene,
    object,
    usd_stage,
    window_manager,
)

register_classes, unregister_classes = bpy.utils.register_classes_factory((
    scene.FinalRenderSettings,
    scene.ViewportRenderSettings,
    scene.SceneProperties,

    object.ObjectProperties,

    usd_stage.UsdStagePrim,
    usd_stage.UsdStage,

    window_manager.WindowManagerProperties,
))


def register():
    register_classes()


def unregister():
    unregister_classes()
