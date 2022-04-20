# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import re

import bpy
# from pxr import Usd, UsdGeom, UsdSkel, Sdf, Tf, Gf

from .base_node import USDNode
from .input import (
    HDUSD_USD_NODETREE_OP_blender_data_link_object, HDUSD_USD_NODETREE_OP_blender_data_unlink_object)

MAX_INPUTS_NUMBER = 10


class HDUSD_USD_NODETREE_MT_instancing_object(bpy.types.Menu):
    bl_idname = "HDUSD_USD_NODETREE_MT_instancing_object"
    bl_label = "Object"
    bl_description = "Object for scattering instances"

    def draw(self, context):
        layout = self.layout
        objects = context.scene.objects

        for obj in objects:
            if obj.hdusd.is_usd or obj.type not in "MESH":
                continue

            row = layout.row()
            op = row.operator(HDUSD_USD_NODETREE_OP_blender_data_link_object.bl_idname,
                              text=obj.name)
            op.object_name = obj.name


class FilterNode(USDNode):
    """Takes in USD and filters out matching path or names"""
    bl_idname = 'usd.FilterNode'
    bl_label = "Filter"
    bl_icon = "FILTER"

    def update_data(self, context):
        self.reset()

    filter_path: bpy.props.StringProperty(
        name="Pattern",
        description="USD Path pattern. Use special characters means:\n"
                    "  * - any word or subword\n"
                    "  ** - several words separated by '/' or subword",
        default='/*',
        update=update_data
    )

    def draw_buttons(self, context, layout):
        layout.prop(self, 'filter_path')

    def compute(self, **kwargs):
        # input_stage = self.get_input_link('Input', **kwargs)
        # if not input_stage:
        #     return None
        #
        # # creating search regex pattern and getting filtered rpims
        # prog = re.compile(self.filter_path.replace('*', '#')        # temporary replacing '*' to '#'
        #                                   .replace('/', '\/')       # for correct regex pattern
        #                                   .replace('##', '[\w\/]*') # creation
        #                                   .replace('#', '\w*'))
        #
        # def get_child_prims(prim):
        #     if not prim.IsPseudoRoot() and prog.fullmatch(str(prim.GetPath())):
        #         yield prim
        #         return
        #
        #     for child in prim.GetAllChildren():
        #         yield from get_child_prims(child)
        #
        # prims = tuple(get_child_prims(input_stage.GetPseudoRoot()))
        # if not prims:
        #     return None
        #
        # stage = self.cached_stage.create()
        # UsdGeom.SetStageMetersPerUnit(stage, 1)
        # UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        #
        # root_prim = stage.GetPseudoRoot()
        #
        # for i, prim in enumerate(prims, 1):
        #     override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        #     override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath,
        #                                                prim.GetPath())
        #
        # return stage
        return None


class MergeNode(USDNode):
    """Merges two USD streams"""
    bl_idname = 'usd.MergeNode'
    bl_label = "Merge"
    bl_icon = "SELECT_EXTEND"

    input_names = tuple(f"Input {i + 1}" for i in range(MAX_INPUTS_NUMBER))

    def update_inputs_number(self, context):
        for i in range(MAX_INPUTS_NUMBER):
            self.inputs[i].hide = i >= self.inputs_number

    def set_inputs_number(self, value):
        max_i = max((i if input.is_linked else 0) for i, input in enumerate(self.inputs)) + 1
        self["inputs_number"] = value if value > max_i else max_i

    def get_inputs_number(self):
        return self.get("inputs_number", 2)

    inputs_number: bpy.props.IntProperty(
        name="Inputs",
        min=2, max=MAX_INPUTS_NUMBER, default=2,
        update=update_inputs_number, set=set_inputs_number, get=get_inputs_number
    )

    def init(self, context):
        super().init(context)
        self.update_inputs_number(context)

    def draw_buttons(self, context, layout):
        layout.prop(self, 'inputs_number')

    def compute(self, **kwargs):
        # ref_stages = []
        # for i in range(len(self.inputs)):
        #     stage = self.get_input_link(i, **kwargs)
        #     if stage:
        #         ref_stages.append(stage)
        #
        # if not ref_stages:
        #     return None
        #
        # if len(ref_stages) == 1:
        #     return ref_stages[0]
        #
        # stage = self.cached_stage.create()
        # UsdGeom.SetStageMetersPerUnit(stage, 1)
        # UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        #
        # root_prim = stage.GetPseudoRoot()
        #
        # for ref_stage in ref_stages:
        #     for prim in ref_stage.GetPseudoRoot().GetAllChildren():
        #         override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        #         override_prim.GetReferences().AddReference(ref_stage.GetRootLayer().realPath, prim.GetPath())
        #
        # return stage
        return None


class RootNode(USDNode):
    """Create root primitive and make it parent for USD primitives"""
    bl_idname = 'usd.RootNode'
    bl_label = "Root"
    bl_icon = "COLLECTION_NEW"

    def update_data(self, context):
        self.reset()

    name: bpy.props.StringProperty(
        name="Name",
        description="Name for USD root primitive",
        default="Root",
        update=update_data
    )
    type:  bpy.props.EnumProperty(
        name="Type",
        description="Filter by type for USD primitives",
        items=(('Xform', "Xform", "Xform primitive type"),
               ('Scope', "Scope", "Scope primitive type"),
               ('SkelRoot', "SkelRoot", "SkelRoot primitive type"),
               ('None', "None", "No primitive type")),
        default='Xform',
        update=update_data
    )

    def draw_buttons(self, context, layout):
        layout.prop(self, 'name')
        layout.prop(self, 'type')

    def compute(self, **kwargs):
        # input_stage = self.get_input_link('Input', **kwargs)
        #
        # if not input_stage:
        #     return None
        #
        # if not self.name:
        #     return input_stage
        #
        # path = f'/{Tf.MakeValidIdentifier(self.name)}'
        # stage = self.cached_stage.create()
        # UsdGeom.SetStageMetersPerUnit(stage, 1)
        # UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        #
        # # create new root prim according to name and type
        # if self.type == 'Xform':
        #     root_prim = UsdGeom.Xform.Define(stage, path)
        # elif self.type == 'Scope':
        #     root_prim = UsdGeom.Scope.Define(stage, path)
        # elif self.type == 'SkelRoot':
        #     root_prim = UsdSkel.Root.Define(stage, path)
        # else:
        #     root_prim = stage.DefinePrim(path)
        #
        # for prim in input_stage.GetPseudoRoot().GetAllChildren():
        #     override_prim = stage.OverridePrim(root_prim.GetPath().AppendChild(prim.GetName()))
        #     override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath, prim.GetPath())
        #
        # return stage
        return None


class InstancingNode(USDNode):
    """Create and distribute instances of primitives"""
    bl_idname = 'usd.InstancingNode'
    bl_label = "Instancing"
    bl_icon = "STICKY_UVS_DISABLE"

    def update_data(self, context):
        self.reset(True)

    name: bpy.props.StringProperty(
        name="Name",
        description="Name for USD instance primitive",
        default="Instance",
        update=update_data
    )

    object: bpy.props.PointerProperty(
        type=bpy.types.Object,
        name="Object",
        description="Object for scattering instances",
        update=update_data
    )

    method: bpy.props.EnumProperty(
        name="Method",
        description="Object instancing method",
        items=(('VERTICES', "Vertices", "Instancing by vertices"),
               ('POLYGONS', "Faces", "Instancing by faces")),
        default='VERTICES',
        update=update_data
    )

    object_transform: bpy.props.BoolProperty(
        name="Use Object Transform",
        default=True,
        update=update_data,
        description="Apply object transform to instances",
    )

    def draw_buttons(self, context, layout):
        layout.prop(self, 'name')

        split = layout.row(align=True).split(factor=0.25)
        col = split.column()

        col.label(text="Object")
        col = split.column()
        row = col.row(align=True)
        if self.object:
            row.menu(HDUSD_USD_NODETREE_MT_instancing_object.bl_idname,
                     text=self.object.name, icon='OBJECT_DATAMODE')
            row.operator(HDUSD_USD_NODETREE_OP_blender_data_unlink_object.bl_idname, icon='X')
            layout.prop(self, 'method')
        else:
            row.menu(HDUSD_USD_NODETREE_MT_instancing_object.bl_idname,
                     text=" ", icon='OBJECT_DATAMODE')

        row = layout.row()
        row.alignment = 'LEFT'
        row.prop(self, 'object_transform')

    def compute(self, **kwargs):
        # if not self.object:
        #     return None
        #
        # input_stage = self.get_input_link('Input', **kwargs)
        #
        # if not input_stage:
        #     return None
        #
        # if not input_stage.GetPseudoRoot().GetAllChildren():
        #     return None
        #
        # depsgraph = bpy.context.evaluated_depsgraph_get()
        # obj = self.object.evaluated_get(depsgraph)
        # distribute_items = obj.data.vertices if self.method == 'VERTICES' else obj.data.polygons
        # if not distribute_items:
        #     return None
        #
        # stage = self.cached_stage.create()
        # UsdGeom.SetStageMetersPerUnit(stage, 1)
        # UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.z)
        #
        # for i, item in enumerate(distribute_items):
        #     root_xform = UsdGeom.Xform.Define(stage, f'/{Tf.MakeValidIdentifier(f"{self.name}_{i}")}')
        #     for prim in input_stage.GetPseudoRoot().GetAllChildren():
        #         override_prim = stage.OverridePrim(root_xform.GetPath().AppendChild(prim.GetName()))
        #         override_prim.GetReferences().AddReference(input_stage.GetRootLayer().realPath, prim.GetPath())
        #
        #     trans = Matrix.Translation(item.co if self.method == 'VERTICES' else item.center)
        #     rot = item.normal.to_track_quat().to_matrix().to_4x4()
        #
        #     transform = trans @ rot
        #     if self.object_transform:
        #         transform = obj.matrix_world @ transform
        #
        #     UsdGeom.Xform.Get(stage, root_xform.GetPath()).MakeMatrixXform()
        #     root_xform.GetPrim().GetAttribute('xformOp:transform').Set(Gf.Matrix4d(transform.transposed()))
        #
        # return stage
        return None

    def depsgraph_update(self, depsgraph):
        if not self.object:
            return

        obj = next((update.id for update in depsgraph.updates if isinstance(update.id, bpy.types.Object)
                    and not update.id.hdusd.is_usd and update.id.name == self.object.name), None)
        if obj:
            self.reset()
