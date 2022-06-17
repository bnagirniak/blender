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

        session_free(self.session)

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
            self.session = session_create(self)

        session_reset(self.session, data, depsgraph, stage)

    def render(self, depsgraph):
        if not self.session:
            return

        session_render(self.session, depsgraph)

    def render_frame_finish(self):
        pass

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
            self.session = session_create(self)

        session_reset(self.session, data, depsgraph, stage)

    def view_draw(self, context, depsgraph):
        if not self.session:
            return

        session_view_draw(self.session, depsgraph, context, context.space_data, context.region_data)


def session_create(engine: HdUSDEngine):
    return _hdusd.session.create(engine.as_pointer())


def session_free(session):
    _hdusd.session.free(session)


def session_reset(session, data, depsgraph, stage):
    _hdusd.session.reset(session, data.as_pointer(), depsgraph.as_pointer(), stage)


def session_render(session, depsgraph):
    _hdusd.session.render(session, depsgraph.as_pointer())


def session_view_draw(session, depsgraph, context, space_data, region_data):
    _hdusd.session.view_draw(session, depsgraph.as_pointer(), context.as_pointer(),
                             space_data.as_pointer(), region_data.as_pointer())
