# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from .engine import StormHydraRenderEngine


class Panel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'render'
    COMPAT_ENGINES = {StormHydraRenderEngine.bl_idname}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class STORM_HYDRA_RENDER_PT_render_settings(Panel):
    """Final render delegate and settings"""
    bl_label = "Storm Render Settings"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        settings = context.scene.hydra_storm
        layout.prop(settings, 'enable_tiny_prim_culling')
        layout.prop(settings, 'max_lights')


class STORM_HYDRA_LIGHT_PT_light(Panel):
    """
    Physical light sources
    """
    bl_label = "Light"
    bl_context = 'data'

    def draw(self, context):
        layout = self.layout

        light = context.light

        layout.prop(light, "type", expand=True)

        layout.use_property_split = True
        layout.use_property_decorate = False

        main_col = layout.column()

        main_col.prop(light, "color")
        main_col.prop(light, "energy")
        main_col.separator()

        if light.type == 'POINT':
            row = main_col.row(align=True)
            row.prop(light, "shadow_soft_size", text="Radius")

        elif light.type == 'SPOT':
            col = main_col.column(align=True)
            col.prop(light, 'spot_size', slider=True)
            col.prop(light, 'spot_blend', slider=True)

            main_col.prop(light, 'show_cone')

        elif light.type == 'SUN':
            main_col.prop(light, "angle")

        elif light.type == 'AREA':
            main_col.prop(light, "shape", text="Shape")
            sub = main_col.column(align=True)

            if light.shape in {'SQUARE', 'DISK'}:
                sub.prop(light, "size")
            elif light.shape in {'RECTANGLE', 'ELLIPSE'}:
                sub.prop(light, "size", text="Size X")
                sub.prop(light, "size_y", text="Y")

            else:
                main_col.prop(light, 'size')
