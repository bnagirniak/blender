# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
from .base_node import USDNode
# from . import log


class WriteFileNode(USDNode):
    """Writes stream out to USD file"""
    bl_idname = 'usd.WriteFileNode'
    bl_label = "Write USD File"
    bl_icon = "FILE_TICK"

    file_path: bpy.props.StringProperty(name="USD File", subtype='FILE_PATH')

    def draw_buttons(self, context, layout):
        layout.prop(self, 'file_path')

    def compute(self, **kwargs):
        # stage = self.get_input_link('Input', **kwargs)
        #
        # if stage and self.file_path:
        #     file_path = bpy.path.abspath(self.file_path)
        #     stage.Export(file_path)
        #
        # return stage
        return None


class HydraRenderNode(USDNode):
    """Render to Hydra"""

    bl_idname = 'usd.HydraRenderNode'
    bl_label = "Render USD via Hydra"
    bl_icon = "RESTRICT_RENDER_OFF"

    output_name = ""

    render_type: bpy.props.EnumProperty(
        name='Type',
        items=(('FINAL', 'Final', 'Final Render'),
               ('VIEWPORT', 'Viewport', 'Viewport Render'),
               ('BOTH', 'Both', 'All Renders'),
               ),
        default='BOTH'
    )

    def compute(self, **kwargs):
        stage = self.get_input_link('Input', **kwargs)
        return stage

    def node_computed(self):
        # notify USD nodetree that Output node was computed
        nodetree = self.id_data
        if self.name == nodetree.get_output_node().name:
            nodetree.output_node_computed()
