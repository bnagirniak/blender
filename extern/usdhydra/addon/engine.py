# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _usdhydra

from . import ui

from . import logger
log = logger.Log('engine')


class HydraRenderEngine(bpy.types.RenderEngine):
    bl_idname = ''
    bl_label = ""
    bl_info = ""

    bl_use_preview = True

    delegate_name = ''
    session = None

    def __del__(self):
        if not self.session:
            return

        session_free(self.session)

    def get_delegate_settings(self, engine_type):
        return {}

    # final render
    def update(self, data, depsgraph):
        engine_type = 'PREVIEW' if self.is_preview else 'FINAL'
        log("update", self, engine_type)

        self.session = session_create(self, engine_type, self.delegate_name)
        delegate_settings = self.get_delegate_settings(engine_type)
        session_sync(self.session, depsgraph, delegate_settings)

    def render(self, depsgraph):
        log("render", self)

        session_render(self.session)

    # viewport render
    def view_update(self, context, depsgraph):
        log("view_update", self)

        if not self.session:
            self.session = session_create(self, 'VIEWPORT', self.delegate_name)

        delegate_settings = self.get_delegate_settings('VIEWPORT')
        session_sync(self.session, depsgraph, delegate_settings)

    def view_draw(self, context, depsgraph):
        if not self.session:
            return

        log("view_draw", self)
        session_view_draw(self.session, depsgraph, context, context.space_data, context.region_data)

    @classmethod
    def register(cls):
        log("register", cls)
        ui.register_engine(cls.bl_idname)

    @classmethod
    def unregister(cls):
        log("unregister", cls)
        ui.unregister_engine(cls.bl_idname)


def session_create(engine: HydraRenderEngine, engine_type, delegate_name):
    return _usdhydra.session.create(engine.as_pointer(), engine_type, delegate_name)


def session_free(session):
    _usdhydra.session.free(session)


def session_render(session):
    _usdhydra.session.render(session)


def session_sync(session, depsgraph, delegate_settings):
    _usdhydra.session.sync(session, depsgraph.as_pointer(), delegate_settings)


def session_view_draw(session, depsgraph, context, space_data, region_data):
    _usdhydra.session.view_draw(session, depsgraph.as_pointer(), context.as_pointer(),
                                space_data.as_pointer(), region_data.as_pointer())


def session_get_render_plugins():
    return _usdhydra.session.get_render_plugins()
