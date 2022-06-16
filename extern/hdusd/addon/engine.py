# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _hdusd

from .usd_nodes import node_tree
from .utils import stages


def init():
    _hdusd.init()


def exit():
    _hdusd.exit()


class HdUSDEngine(bpy.types.RenderEngine):
    bl_idname = 'HdUSD'
    bl_label = "USD Hydra"
    bl_info = "USD Hydra rendering plugin"

    bl_use_preview = True
    bl_use_shading_nodes = True
    bl_use_shading_nodes_custom = False
    bl_use_gpu_context = True

    session = None

    def __init__(self):
        self.session = None

    def __del__(self):
        if not self.session:
            return

        _hdusd.free(self.session)

    # final render
    def update(self, data, depsgraph):
        usd_nodetree = node_tree.get_usd_nodetree()
        if not usd_nodetree:
            return

        output_node = usd_nodetree.output_node
        if not output_node:
            return

        stage = stages.get(output_node)
        if not stage:
            return

        if not self.session:
            self.session = _hdusd.create(self.as_pointer())

        _hdusd.reset(self.session, data.as_pointer(), depsgraph.as_pointer(), stage)

    def render(self, depsgraph):
        if not self.session:
            return

        _hdusd.render(self.session, depsgraph.as_pointer())

    def render_frame_finish(self):
        if not self.session:
            return

        _hdusd.render_frame_finish(self.session)

    # viewport render
    def view_update(self, context, depsgraph):
        data = context.blend_data
        usd_nodetree = node_tree.get_usd_nodetree()
        if not usd_nodetree:
            return

        output_node = usd_nodetree.output_node
        if not output_node:
            return

        stage = stages.get(output_node)
        if not stage:
            return

        if not self.session:
            self.session = _hdusd.create(self.as_pointer())

        _hdusd.reset(self.session, data.as_pointer(), depsgraph.as_pointer(), stage)

    def view_draw(self, context, depsgraph):
        if not self.session:
            return

        depsgraph_ptr = depsgraph.as_pointer()
        space_data_ptr = context.space_data.as_pointer()
        region_data_ptr = context.region_data.as_pointer()
        context_ptr = context.as_pointer()

        _hdusd.view_draw(self.session, depsgraph_ptr, context_ptr, space_data_ptr, region_data_ptr)
