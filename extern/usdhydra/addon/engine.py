# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import sys

import bpy
import _usdhydra

from .usd_nodes import node_tree
from .utils import stages, logging
log = logging.Log('engine')

IS_MATERIALX_ADDON_LOADED = False

try:
    import materialx
    IS_MATERIALX_ADDON_LOADED = True

except Exception as e:
    log.warn("MaterialX Addon isn't loaded")


def exit():
    _usdhydra.exit()


class USDHydraEngine(bpy.types.RenderEngine):
    bl_idname = 'USDHydra'
    bl_label = "USD Hydra Internal"
    bl_info = "USD Hydra rendering plugin"

    bl_use_preview = True
    bl_use_shading_nodes = True
    bl_use_shading_nodes_custom = False
    bl_use_gpu_context = True

    session = None
    delegate_name = "HdStormRendererPlugin"
    materialx_data = []

    def __init__(self):
        self.session = None

    def __del__(self):
        if not self.session:
            return

        session_free(self.session)

    # final render
    def update(self, data, depsgraph):
        is_blender_scene = not bool(depsgraph.scene.usdhydra.final.data_source)
        stage = 0

        if not is_blender_scene:
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

        self.bl_use_gpu_context = self.delegate_name == "HdRprPlugin"

        self.get_materialx_data(data, depsgraph)

        session_reset(self.session, data, bpy.context, depsgraph, self.materialx_data, is_blender_scene,
                      stage, self.delegate_name, self.is_preview)
        session_final_update(self.session, depsgraph)

    def render(self, depsgraph):
        if not self.session:
            return

        delegate_settings = self.sync_final_delegate_settings()

        session_render(self.session, depsgraph, self.delegate_name, delegate_settings)

    def render_frame_finish(self):
        pass

    # viewport render
    def view_update(self, context, depsgraph):
        data = context.blend_data
        is_blender_scene = not bool(context.scene.usdhydra.viewport.data_source)
        stage = 0

        if not is_blender_scene:
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

        delegate_settings = self.sync_viewport_delegate_settings()
        self.get_materialx_data(context, depsgraph)

        session_reset(self.session, data, context, depsgraph, self.materialx_data, is_blender_scene,
                      stage, self.delegate_name, self.is_preview)
        session_view_update(self.session, depsgraph, context, context.space_data, context.region_data, self.delegate_name, delegate_settings)

    def view_draw(self, context, depsgraph):
        if not self.session:
            return

        session_view_draw(self.session, depsgraph, context, context.space_data, context.region_data)

    def sync_final_delegate_settings(self):
        return tuple()

    def sync_viewport_delegate_settings(self):
        return tuple()

    def get_materialx_data(self, context, depsgraph):
        if not IS_MATERIALX_ADDON_LOADED:
            return

        for obj in bpy.context.scene.objects:
            if obj.type in ('EMPTY', 'ARMATURE', 'LIGHT', 'CAMERA'):
                continue

            for mat_slot in obj.material_slots:
                if not mat_slot:
                    continue

                material = mat_slot.material
                matx_data = next((mat for mat in self.materialx_data if mat[0] == material.name), None)

                if not matx_data:
                    mx_file, doc = material.materialx.get_materialx_data(obj)
                    surfacematerial = next((node for node in doc.getNodes() if node.getCategory() == 'surfacematerial'))

                    self.materialx_data.append((material.name, str(mx_file), surfacematerial.getName()))

        materialx.utils.update_materialx_data(depsgraph, self.materialx_data)


class USDHydraHdStormEngine(USDHydraEngine):
    bl_idname = 'USDHydraHdStormRendererPlugin'
    bl_label = "USD Hydra: GL"
    bl_info = "USD Hydra HdStormRendererPlugin rendering plugin"

    bl_use_preview = False
    bl_use_shading_nodes = True
    bl_use_shading_nodes_custom = False
    bl_use_gpu_context = True

    delegate_name = "HdStormRendererPlugin"


def session_create(engine: USDHydraEngine):
    return _usdhydra.session.create(engine.as_pointer())


def session_free(session):
    _usdhydra.session.free(session)


def session_reset(session, data, context, depsgraph, materialx_data, is_blender_scene, stage, delegate, is_preview):
    _usdhydra.session.reset(session, data.as_pointer(), context.as_pointer(), depsgraph.as_pointer(),
                            materialx_data, is_blender_scene, stage, delegate, is_preview)


def session_render(session, depsgraph, delegate, delegate_settings):
    _usdhydra.session.render(session, depsgraph.as_pointer(), delegate, delegate_settings)


def session_final_update(session, depsgraph):
    _usdhydra.session.final_update(session, depsgraph.as_pointer())


def session_view_draw(session, depsgraph, context, space_data, region_data):
    _usdhydra.session.view_draw(session, depsgraph.as_pointer(), context.as_pointer(),
                                space_data.as_pointer(), region_data.as_pointer())


def session_view_update(session, depsgraph, context, space_data, region_data, delegate, delegate_settings):
    _usdhydra.session.view_update(session, depsgraph.as_pointer(), context.as_pointer(),
                                  space_data.as_pointer(), region_data.as_pointer(),
                                  delegate, delegate_settings)


def session_get_render_plugins():
    return _usdhydra.session.get_render_plugins()
