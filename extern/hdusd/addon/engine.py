# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _hdusd


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
    bl_use_gpu_context = False

    def __init__(self):
        self.session = None

    def __del__(self):
        if not self.session:
            return

        _hdusd.free(self.session)

    # final render
    def update(self, data, depsgraph):
        if not self.session:
            self.session = _hdusd.create(self, data.as_pointer())

        _hdusd.reset(self.session, data.as_pointer(), depsgraph.as_pointer())

    def render(self, depsgraph):
        _hdusd.render(self.session, depsgraph.as_pointer())

    def render_frame_finish(self):
        _hdusd.render_frame_finish(self.session)

    # viewport render
    def view_update(self, context, depsgraph):
        data = context.blend_data
        if not self.session:
            self.session = _hdusd.create(self, data.as_pointer())

        _hdusd.reset(self.session, data.as_pointer(), depsgraph.as_pointer())

    def view_draw(self, context, depsgraph):
        depsgraph_ptr = depsgraph.as_pointer()
        space_data_ptr = context.space_data.as_pointer()
        region_data_ptr = context.region_data.as_pointer()

        _hdusd.view_draw(self.session, depsgraph_ptr, space_data_ptr, region_data_ptr)
