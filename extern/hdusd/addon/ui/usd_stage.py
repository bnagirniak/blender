# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import HdUSD_Panel, HdUSD_Operator
from ..usd_nodes.nodes.base_node import USDNode
from ..properties.usd_stage import get_stage_properties


class HDUSD_OP_usd_stage_prim_expand(bpy.types.Operator):
    """Expand USD item"""
    bl_idname = "hdusd.usd_stage_prim_expand"
    bl_label = "Expand"

    index: bpy.props.IntProperty(default=-1)

    def execute(self, context):
        if self.index == -1:
            return {'CANCELLED'}

        node = context.active_node
        usd_list = node.hdusd.usd_list
        items = usd_list.items
        item = items[self.index]

        if len(items) > self.index + 1 and items[self.index + 1].indent > item.indent:
            next_index = self.index + 1
            item_indent = item.indent
            removed_items = 0
            while True:
                if next_index >= len(items):
                    break
                if items[next_index].indent <= item_indent:
                    break
                items.remove(next_index)
                removed_items += 1

            if usd_list.item_index > self.index:
                usd_list.item_index = max(self.index, usd_list.item_index - removed_items)

        else:
            prim = usd_list.get_prim(item)

            added_items = 0
            for child_index, child_prim in enumerate(prim.GetChildren(), self.index + 1):
                child_item = items.add()
                child_item.sdf_path = str(child_prim.GetPath())
                items.move(len(items) - 1, child_index)
                added_items += 1

            if usd_list.item_index > self.index:
                usd_list.item_index += added_items

        return {'FINISHED'}


# class HDUSD_OP_usd_stage_prim_show_hide(bpy.types.Operator):
#     """Show/Hide USD item"""
#     bl_idname = "hdusd.usd_stage_prim_show_hide"
#     bl_label = "Show/Hide"
#
#     index: bpy.props.IntProperty(default=-1)
#
#     def execute(self, context):
#         if self.index == -1:
#             return {'CANCELLED'}
#
#         node = context.active_node
#         usd_list = node.hdusd.usd_list
#         items = usd_list.items
#         item = items[self.index]
#
#         prim = usd_list.get_prim(item)
#         im = UsdGeom.Imageable(prim)
#         if im.ComputeVisibility() == 'invisible':
#             im.MakeVisible()
#         else:
#             im.MakeInvisible()
#
#         return {'FINISHED'}


class HDUSD_UL_usd_stage(bpy.types.UIList):
    def draw_item(self, context, layout, stage_prop, prim_prop, icon, active_data, active_propname, index):
        if self.layout_type not in {'DEFAULT', 'COMPACT'}:
            return

        for i in range(prim_prop.indent):
            layout.split(factor=0.1)

        prims = stage_prop.prims
        prim = data.get_prim(item)
        if not prim:
            return

        visible = UsdGeom.Imageable(prim).ComputeVisibility() != 'invisible'

        col = layout.column()
        if not prim.GetChildren():
            icon = 'DOT'
            col.enabled = False
        elif len(prims) > index + 1 and prims[index + 1].indent > prim_prop.indent:
            icon = 'TRIA_DOWN'
        else:
            icon = 'TRIA_RIGHT'

        expand_op = col.operator(HDUSD_OP_usd_stage_prim_expand.bl_idname, text="", icon=icon,
                                 emboss=False, depress=False)
        expand_op.index = index

        col = layout.column()
        col.label(text=prim.GetName())
        col.enabled = visible

        col = layout.column()
        col.alignment = 'RIGHT'
        col.label(text=prim.GetTypeName())
        col.enabled = visible

        # col = layout.column()
        # col.alignment = 'RIGHT'
        # if prim.GetTypeName() == 'Xform':
        #     icon = 'HIDE_OFF' if visible else 'HIDE_ON'
        # else:
        #     col.enabled = False
        #     icon = 'NONE'
        #
        # visible_op = col.operator(HDUSD_OP_usd_stage_prim_show_hide.bl_idname, text="", icon=icon,
        #                           emboss=False, depress=False)
        # visible_op.index = index
