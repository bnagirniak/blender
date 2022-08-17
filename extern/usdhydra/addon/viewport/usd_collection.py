# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _usdhydra

from pxr import Sdf

from .. import handlers
from ..properties.object import GEOM_TYPES
from ..utils import stages

from ..utils import logging
log = logging.Log('usd_collection')


COLLECTION_NAME = "USD NodeTree"
USD_CAMERA = "USD Camera"


def ignore_prim(prim: dict):
    prim_type = prim['type']
    if not prim_type:
        if prim['name'] == '_materials':
            return True

        return False

    

    return not (prim_type in GEOM_TYPES or prim_type in ('Mesh', 'Camera') or prim_type.endswith('Light'))


def update(context):
    def update_():
        usd_tree_name = context.scene.usdhydra.viewport.data_source
        if not usd_tree_name:
            clear(context)
            return

        usd_nodetree = bpy.data.node_groups[usd_tree_name]
        output_node = usd_nodetree.output_node
        if not output_node:
            clear(context)
            return

        stage = stages.get(output_node)
        if not stage:
            clear(context)
            return

        # workaround for Undo operation - Blender doesn't send bpy.data.scenes and bpy.data.collections
        # so we need to do nothing to prevent Blender crash
        if len(bpy.data.scenes) == 0:
            return

        collection = bpy.data.collections.get(COLLECTION_NAME)
        if not collection:
            collection = bpy.data.collections.new(COLLECTION_NAME)
            context.scene.collection.children.link(collection)
            log("Collection created", collection)

        objects = {}
        for obj in collection.objects:
            if obj.usdhydra.is_usd:
                objects[obj.usdhydra.sdf_path] = obj
        obj_paths = set(objects.keys())

        prim_paths = set()
        for prim in _usdhydra.stage.traverse_stage(stage):
            if not ignore_prim(prim):
                prim_paths.add(prim['path'])

        paths_to_remove = obj_paths - prim_paths
        paths_to_add = prim_paths - obj_paths
        path_to_update = obj_paths.intersection(prim_paths)

        log(f"Removing {len(paths_to_remove)} objects")
        for path in paths_to_remove:
            obj = objects.pop(path)
            bpy.data.objects.remove(obj)

        log(f"Updated {len(path_to_update)} objects")
        for path in path_to_update:
            prim_info = _usdhydra.stage.prim_get_info(stage, path)
            if prim_info['type'] in GEOM_TYPES:
                objects[path].usdhydra.sync_transform_from_prim(stage, path)

        log(f"Adding {len(paths_to_add)} objects")
        for path in sorted(paths_to_add):
            parent_path = str(Sdf.Path(path).GetParentPath())
            parent_obj = None if parent_path == '/' else objects[parent_path]
        
            obj = bpy.data.objects.new('/', None)

            obj.usdhydra.sync_from_prim(parent_obj, stage, path)
            collection.objects.link(obj)
        
            objects[path] = obj

    handlers.no_depsgraph_update_call(update_)


def clear(context):
    def clear_():
        collection = bpy.data.collections.get(COLLECTION_NAME)
        if not collection:
            return

        log("Removing collection", collection)
        for obj in collection.objects:
            if obj.usdhydra.is_usd:
                bpy.data.objects.remove(obj)

        bpy.data.collections.remove(collection)

    handlers.no_depsgraph_update_call(clear_)


def scene_save_pre():
    context = bpy.context
    clear(context)


def scene_save_post():
    context = bpy.context
    update(context)
