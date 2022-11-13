# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>
import sys
import importlib

import bpy
import addon_utils
import _usdhydra

import ui

import logger
log = logger.Log('engine')


def exit():
    _usdhydra.exit()


class HydraRenderEngine(bpy.types.RenderEngine):
    bl_idname = ''
    bl_label = ""
    bl_info = ""

    bl_use_preview = True
    bl_use_shading_nodes = True
    bl_use_shading_nodes_custom = False

    delegate_name = ""
    session = None

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
        _, matx_enabled = addon_utils.check('materialx')
        if not matx_enabled:
            log.warn("MaterialX Addon isn't loaded")
            return

        import materialx

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

    @classmethod
    def register(cls):
        log("Register", cls)
        ui.register_engine(cls.bl_idname)

    @classmethod
    def unregister(cls):
        log("Unregister", cls)
        ui.unregister_engine(cls.bl_idname)


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


RENDER_DELEGATE_ADDONS = set()


def register_delegate(delegate_dir, engine_bl_idname):
    import _usdhydra
    from ..ui import USDHydra_Panel, USDHydra_Operator
    from ..ui.panels import get_panels
    from ..usd_nodes.node_tree import USDTree

    global RENDER_DELEGATE_ADDONS

    _usdhydra.init_delegate(str(delegate_dir))

    for panel in get_panels():
        panel.COMPAT_ENGINES.add(engine_bl_idname)

    USDHydra_Panel.COMPAT_ENGINES.add(engine_bl_idname)
    USDHydra_Operator.COMPAT_ENGINES.add(engine_bl_idname)
    USDTree.COMPAT_ENGINES.add(engine_bl_idname)
    RENDER_DELEGATE_ADDONS.add(engine_bl_idname)


def unregister_delegate(engine_bl_idname):
    from ..ui import USDHydra_Panel, USDHydra_Operator
    from ..ui.panels import get_panels
    from ..usd_nodes.node_tree import USDTree

    try:
        USDHydra_Panel.COMPAT_ENGINES.remove(engine_bl_idname)
        USDHydra_Operator.COMPAT_ENGINES.remove(engine_bl_idname)

        for panel in get_panels():
            if 'USDHydraHdStormRendererPlugin' in panel.COMPAT_ENGINES:
                panel.COMPAT_ENGINES.remove(engine_bl_idname)

        USDTree.COMPAT_ENGINES.remove(engine_bl_idname)

    except:
        pass


def disable_delegates():
    for delegate in RENDER_DELEGATE_ADDONS:
        enabled, loaded = addon_utils.check(delegate)
        if enabled:
            log.warn("Disable Delegate ", delegate)
            addon_utils.disable(delegate)
            bpy.ops.preferences.addon_disable(module=delegate)


def enable_delegates():
    for delegate in RENDER_DELEGATE_ADDONS:
        enabled, loaded = addon_utils.check(delegate)
        if not loaded or not loaded:
            mod = sys.modules.get(delegate)
            importlib.reload(mod)
            addon_utils.enable(delegate)
