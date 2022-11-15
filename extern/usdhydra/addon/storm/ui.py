# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


class Panel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'render'
    COMPAT_ENGINES = {'HdStormHydraRenderEngine'}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class USDHYDRA_STORM_RENDER_PT_render_settings(Panel):
    """Final render delegate and settings"""
    bl_label = "Render Settings"

    def draw(self, context):
        layout = self.layout
        layout.use_property_split = True
        layout.use_property_decorate = False

        settings = context.scene.usdhydra_storm
        layout.prop(settings, 'enable_tiny_prim_culling')
        layout.prop(settings, 'max_lights')



register, unregister = bpy.utils.register_classes_factory((
    USDHYDRA_STORM_RENDER_PT_render_settings,
))

