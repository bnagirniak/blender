# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from ..utils import stages


class UsdStagePrim(bpy.types.PropertyGroup):
    path: bpy.props.StringProperty(name='USD Path', default="")

    @property
    def indent(self):
        return self.path.count('/') - 1


class UsdStage(bpy.types.PropertyGroup):
    prims: bpy.props.CollectionProperty(type=UsdStagePrim)
    prim_index: bpy.props.IntProperty(name="USD Item", default=-1)

    @property
    def stage(self):
        return stages.get_by_key(self.name)

    def update_prims(self):
        self.prims.clear()
        self.prims_index = -1

    #     stage = self.cached_stage()
    #     if stage:
    #         for prim in stage.GetPseudoRoot().GetChildren():
    #             item = self.items.add()
    #             item.sdf_path = str(prim.GetPath())

    # def get_prim(self, item):
    #     stage = self.cached_stage()
    #     return stage.GetPrimAtPath(item.sdf_path) if stage else None
    #
    # @property
    # def selected_prim(self):
    #     item = self.items[self.item_index]
    #     return self.get_prim(item)


def get_stage_properties(bl_obj, do_create=True):
    key = str(bl_obj.as_pointer())
    stage_prop_list = bpy.context.window_manager.hdusd.usd_stages
    stage_prop = stage_prop_list.get(key)

    if not stage_prop and do_create:
        stage_prop = stage_prop_list.add()
        stage_prop.name = key

    return stage_prop


def remove_stage_properties(bl_obj):
    key = str(bl_obj.as_pointer())
    stage_prop_list = bpy.context.window_manager.hdusd.usd_stages
    ind = next((i for i, stage_prop in enumerate(stage_prop_list) if stage_prop.name == key), -1)
    stage_prop_list.remove(ind)
