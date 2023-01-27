# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import traceback

import bpy
import _hydra

from _hydra import register_plugins, get_render_plugins


class HydraRenderEngine(bpy.types.RenderEngine):
    delegate_id = ''
    engine_ptr = None

    def __del__(self):
        if not self.engine_ptr:
            return

        _hydra.engine_free(self.engine_ptr)

    @classmethod
    def register(cls):
        _hydra.init()

    @classmethod
    def unregister(cls):
        pass

    def get_delegate_settings(self, engine_type):
        return {}

    # final render
    def update(self, data, depsgraph):
        pass

    def render(self, depsgraph):
        engine_type = 'PREVIEW' if self.is_preview else 'FINAL'

        self.engine_ptr = _hydra.engine_create(self.as_pointer(), engine_type, self.delegate_id)
        delegate_settings = self.get_delegate_settings(engine_type)

        _hydra.engine_sync(self.engine_ptr, depsgraph.as_pointer(), bpy.context.as_pointer(), delegate_settings)
        _hydra.engine_render(self.engine_ptr, depsgraph.as_pointer())

    # viewport render
    def view_update(self, context, depsgraph):
        if not self.engine_ptr:
            self.engine_ptr = _hydra.engine_create(self.as_pointer(), 'VIEWPORT', self.delegate_id)

        delegate_settings = self.get_delegate_settings('VIEWPORT')
        _hydra.engine_sync(self.engine_ptr, depsgraph.as_pointer(), context.as_pointer(), delegate_settings)

    def view_draw(self, context, depsgraph):
        if not self.engine_ptr:
            return

        _hydra.engine_view_draw(self.engine_ptr, depsgraph.as_pointer(), context.as_pointer())


def export_mtlx(material_name):
    try:
        import materialx.utils as mx_utils

        material = bpy.data.materials[material_name]
        doc = mx_utils.export(material, None)
        if not doc:
            return ""

        mtlx_file = mx_utils.get_temp_file(".mtlx", material.name)
        mx_utils.export_to_file(doc, mtlx_file, export_deps=True, copy_deps=False)
        return str(mtlx_file)

    except ImportError:
        print("ERROR: no MaterialX addon available")

    except Exception as e:
        print("ERROR:", e, traceback.format_exc())

    return ""
