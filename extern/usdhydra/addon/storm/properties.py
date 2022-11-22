# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


class Properties(bpy.types.PropertyGroup):
    bl_type = None

    @classmethod
    def register(cls):
        cls.bl_type.usdhydra_storm = bpy.props.PointerProperty(
            name="USDHydra Storm",
            description="USDHydra Storm properties",
            type=cls,
        )

    @classmethod
    def unregister(cls):
        del cls.bl_type.usdhydra_storm


class SceneProperties(Properties):
    bl_type = bpy.types.Scene

    enable_tiny_prim_culling: bpy.props.BoolProperty(
        name="Tiny Prim Culling",
        description="Enable Tiny Prim Culling",
        default=False,
    )
    max_lights: bpy.props.IntProperty(
        name="Max Lights",
        description="Maximum number of lights",
        default=16, min=0,
    )
