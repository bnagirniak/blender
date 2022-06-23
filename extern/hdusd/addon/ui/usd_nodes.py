# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import _hdusd
from . import HdUSD_Panel, HdUSD_ChildPanel, HdUSD_Operator
from ..usd_nodes.nodes.base_node import USDNode

from ..utils.logging import Log
log = Log('ui.usd_nodes')


class HDUSD_NODE_PT_usd_stage(HdUSD_Panel):
    bl_label = "USD Node Prims"
    bl_space_type = "NODE_EDITOR"
    bl_region_type = "UI"
    bl_category = "Item"

    @classmethod
    def poll(cls, context):
        node = context.active_node
        return node and isinstance(node, USDNode)

    def draw(self, context):
        layout = self.layout
        usd_node = context.active_node
        stage_prop = usd_node.stage_prop

        # layout.label(text=f"stage: {stage_prop.stage}")

        layout.template_list(
            "HDUSD_UL_usd_stage", "",
            stage_prop, "prims",
            stage_prop, "prim_index",
            sort_lock=True
        )


class HDUSD_OP_usd_tree_node_print_stage(HdUSD_Operator):
    """ Print selected USD nodetree node stage to console """
    bl_idname = "hdusd.usd_tree_node_print_stage"
    bl_label = "Print Stage To Console"

    @classmethod
    def poll(cls, context):
        return super().poll(context) and context.space_data.tree_type == 'hdusd.USDTree' and \
               context.active_node

    def execute(self, context):
        node = context.active_node

        stage = node.stage
        if not stage:
            log.info(f"No USD stage for node {node.name}")
            return {'CANCELLED'}

        usd_str = _hdusd.stage.export_to_str(stage, True)
        print(usd_str)

        return {'FINISHED'}


class HDUSD_OP_usd_tree_node_print_root_layer(HdUSD_Operator):
    """ Print selected USD nodetree node stage to console """
    bl_idname = "hdusd.usd_tree_node_print_root_layer"
    bl_label = "Print Root Layer To Console"

    @classmethod
    def poll(cls, context):
        return super().poll(context) and context.space_data.tree_type == 'hdusd.USDTree' and \
               context.active_node

    def execute(self, context):
        node = context.active_node

        stage = node.stage
        if not stage:
            log.info(f"No USD stage for node {node.name}")
            return {'CANCELLED'}

        usd_str = _hdusd.stage.export_to_str(stage, False)
        print(usd_str)

        return {'FINISHED'}


class HDUSD_NODE_PT_usd_nodetree_tools(HdUSD_Panel):
    bl_label = "USD Tools"
    bl_space_type = "NODE_EDITOR"
    bl_region_type = "UI"
    bl_category = "Tool"

    @classmethod
    def poll(cls, context):
        tree = context.space_data.edit_tree
        return super().poll(context) and tree and tree.bl_idname == 'hdusd.USDTree'

    def draw(self, context):
        # layout = self.layout
        pass


class HDUSD_NODE_PT_usd_nodetree_dev(HdUSD_ChildPanel):
    bl_label = "Dev"
    bl_parent_id = 'HDUSD_NODE_PT_usd_nodetree_tools'
    bl_space_type = "NODE_EDITOR"
    bl_region_type = "UI"
    bl_category = "Tool"

    @classmethod
    def poll(cls, context):
        from ..properties.preferences import get_addon_pref
        return get_addon_pref().dev_tools

    def draw(self, context):
        layout = self.layout

        layout.operator(HDUSD_OP_usd_tree_node_print_stage.bl_idname)
        layout.operator(HDUSD_OP_usd_tree_node_print_root_layer.bl_idname)
