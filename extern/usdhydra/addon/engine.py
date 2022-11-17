# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _usdhydra

from . import ui
from .preferences import addon_preferences

from . import logger
log = logger.Log('engine')


class HydraRenderEngine(bpy.types.RenderEngine):
    bl_idname = ''
    bl_label = ""
    bl_info = ""

    bl_use_preview = True

    delegate_id = ''
    engine_ptr = None

    def __del__(self):
        if not self.engine_ptr:
            return

        _usdhydra.engine.free(self.engine_ptr)

    def get_delegate_settings(self, engine_type):
        return {}

    # final render
    def update(self, data, depsgraph):
        engine_type = 'PREVIEW' if self.is_preview else 'FINAL'
        log("update", self, engine_type)

        self.engine_ptr = _usdhydra.engine.create(self.as_pointer(), engine_type, self.delegate_id)
        delegate_settings = self.get_delegate_settings(engine_type)
        _usdhydra.engine.sync(self.engine_ptr, depsgraph.as_pointer(), bpy.context.as_pointer(), delegate_settings)

    def render(self, depsgraph):
        log("render", self)
        _usdhydra.engine.render(self.engine_ptr, depsgraph.as_pointer())

    # viewport render
    def view_update(self, context, depsgraph):
        log("view_update", self)

        if not self.engine_ptr:
            self.engine_ptr = _usdhydra.engine.create(self.as_pointer(), 'VIEWPORT', self.delegate_name)

        delegate_settings = self.get_delegate_settings('VIEWPORT')
        _usdhydra.engine.sync(self.engine_ptr, depsgraph.as_pointer(), delegate_settings)

    def view_draw(self, context, depsgraph):
        if not self.engine_ptr:
            return

        log("view_draw", self)
        _usdhydra.engine.view_draw(self.engine_ptr, depsgraph.as_pointer(), context.as_pointer(),
                                   context.space_data.as_pointer(), context.region_data.as_pointer())

    @classmethod
    def register(cls):
        log("register", cls)
        ui.register_engine(cls.bl_idname)

    @classmethod
    def unregister(cls):
        log("unregister", cls)
        ui.unregister_engine(cls.bl_idname)


def register():
    if addon_preferences().storm_render_engine:
        from . import storm
        storm.register()


def unregister():
    if addon_preferences().storm_render_engine:
        from . import storm
        storm.unregister()
