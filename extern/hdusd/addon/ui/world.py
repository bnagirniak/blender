# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

from cycles.ui import panel_node_draw

from . import HdUSD_Panel


class HDUSD_WORLD_PT_preview(HdUSD_Panel):
    bl_label = "Preview"
    bl_context = "world"
    bl_options = {'DEFAULT_CLOSED'}

    @classmethod
    def poll(cls, context):
        return context.world and super().poll(context)

    def draw(self, context):
        self.layout.template_preview(context.world)


class HDUSD_WORLD_PT_surface(HdUSD_Panel):
    bl_label = "Surface"
    bl_context = "world"

    @classmethod
    def poll(cls, context):
        return context.world and super().poll(context)

    def draw(self, context):
        layout = self.layout

        layout.use_property_split = True

        world = context.world

        if not panel_node_draw(layout, world, 'OUTPUT_WORLD', 'Surface'):
            layout.prop(world, "color")

