# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy


@bpy.app.handlers.persistent
def on_load_pre(*args):
    """Handler on loading a blend file (before)"""
    # log("on_load_pre", args)
    # utils.clear_temp_dir()
    pass


@bpy.app.handlers.persistent
def on_load_post(*args):
    """Handler on loading a blend file (after)"""
    # log("on_load_post", args)
    from .usd_nodes import node_tree
    from .utils import stages

    stages.free_all()
    node_tree.reset()


# _do_depsgraph_update = True
#
#
# @bpy.app.handlers.persistent
# def on_depsgraph_update_post(scene, depsgraph):
#     global _do_depsgraph_update
#     if not _do_depsgraph_update:
#         return
#
#     log("on_depsgraph_update", depsgraph)
#     from ..properties import object, material
#     from ..usd_nodes import node_tree
#     from ..ui import material as material_ui
#
#     object.depsgraph_update(depsgraph)
#     material.depsgraph_update(depsgraph)
#     node_tree.depsgraph_update(depsgraph)
#     material_ui.depsgraph_update(depsgraph)
#
#
# def no_depsgraph_update_call(op, *args, **kwargs):
#     """This function prevents call of self.update() during calling our function"""
#     global _do_depsgraph_update
#     if not _do_depsgraph_update:
#         return op(*args, **kwargs)
#
#     _do_depsgraph_update = False
#     try:
#         return op(*args, **kwargs)
#     finally:
#         _do_depsgraph_update = True
#
#
# @bpy.app.handlers.persistent
# def on_frame_change_post(scene, depsgraph):
#     """Handler on frame change a blend file (after)"""
#     log("on_frame_change", depsgraph)
#     from ..usd_nodes import node_tree
#
#     node_tree.frame_change(depsgraph)
#
#
# @bpy.app.handlers.persistent
# def on_save_pre(*args):
#     log("on_save_pre", args)
#     from ..viewport import usd_collection
#     usd_collection.scene_save_pre()
#
#
# @bpy.app.handlers.persistent
# def on_save_post(*args):
#     log("on_save_post", args)
#     from ..viewport import usd_collection
#     usd_collection.scene_save_post()


def register():
    bpy.app.handlers.load_pre.append(on_load_pre)
    bpy.app.handlers.load_post.append(on_load_post)
    # bpy.app.handlers.depsgraph_update_post.append(on_depsgraph_update_post)
    # bpy.app.handlers.frame_change_post.append(on_frame_change_post)
    # bpy.app.handlers.save_pre.append(on_save_pre)
    # bpy.app.handlers.save_post.append(on_save_post)


def unregister():
    bpy.app.handlers.load_pre.remove(on_load_pre)
    bpy.app.handlers.load_post.remove(on_load_post)
    # bpy.app.handlers.depsgraph_update_post.remove(on_depsgraph_update_post)
    # bpy.app.handlers.frame_change_post.remove(on_frame_change_post)
    # bpy.app.handlers.save_pre.remove(on_save_pre)
    # bpy.app.handlers.save_post.remove(on_save_post)
