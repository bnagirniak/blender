# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


class HdUSD_Operator(bpy.types.Operator):
    COMPAT_ENGINES = {'HdUSD'}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class HdUSD_Panel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = 'render'
    COMPAT_ENGINES = {'HdUSD'}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class HdUSD_ChildPanel(bpy.types.Panel):
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_parent_id = ''


from . import (
    panels,
    render,
    hdrpr_render,
    light,
    material,
    world,
    object,
)


register_classes, unregister_classes = bpy.utils.register_classes_factory([
    render.HDUSD_OP_data_source,
    render.HDUSD_OP_nodetree_camera,
    render.HDUSD_MT_data_source_final,
    render.HDUSD_MT_nodetree_camera_final,
    render.HDUSD_MT_nodetree_camera_viewport,
    render.HDUSD_MT_data_source_viewport,
    render.HDUSD_RENDER_PT_render_settings_final,
    render.HDUSD_RENDER_PT_render_settings_viewport,

    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_final,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_samples_final,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_quality_final,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_denoise_final,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_film_final,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_viewport,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_samples_viewport,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_quality_viewport,
    hdrpr_render.HDUSD_RENDER_PT_hdrpr_settings_denoise_viewport,

    light.HDUSD_LIGHT_PT_light,

    material.HDUSD_MATERIAL_PT_context,
    material.HDUSD_MATERIAL_PT_preview,
    material.HDUSD_MATERIAL_OP_new_mx_node_tree,
    material.HDUSD_MATERIAL_OP_duplicate_mx_node_tree,
    material.HDUSD_MATERIAL_OP_convert_shader_to_mx,
    material.HDUSD_MATERIAL_OP_duplicate_mat_mx_node_tree,
    material.HDUSD_MATERIAL_OP_link_mx_node_tree,
    material.HDUSD_MATERIAL_OP_unlink_mx_node_tree,
    material.HDUSD_MATERIAL_MT_mx_node_tree,
    material.HDUSD_MATERIAL_PT_material,
    material.HDUSD_MATERIAL_PT_material_settings_surface,
    material.HDUSD_MATERIAL_OP_link_mx_node,
    material.HDUSD_MATERIAL_OP_invoke_popup_input_nodes,
    material.HDUSD_MATERIAL_OP_invoke_popup_shader_nodes,
    material.HDUSD_MATERIAL_OP_remove_node,
    material.HDUSD_MATERIAL_OP_disconnect_node,
    material.HDUSD_MATERIAL_PT_material_settings_displacement,
    material.HDUSD_MATERIAL_PT_output_surface,
    material.HDUSD_MATERIAL_PT_output_displacement,
    material.HDUSD_MATERIAL_PT_output_volume,
    material.HDUSD_MATERIAL_OP_export_mx_file,
    material.HDUSD_MATERIAL_OP_export_mx_console,
    material.HDUSD_MATERIAL_PT_tools,
    material.HDUSD_MATERIAL_PT_dev,

    world.HDUSD_WORLD_PT_surface,

    object.HDUSD_OBJECT_PT_usd_settings,
    object.HDUSD_OP_usd_object_show_hide,
])


def register():
    panels.register()
    register_classes()


def unregister():
    panels.unregister()
    unregister_classes()
