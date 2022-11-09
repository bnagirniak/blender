# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _usdhydra

from ..viewport import usd_collection
#from ..export.camera import CameraData
#from ..viewport.usd_collection import USD_CAMERA
from . import USDHydraProperties



class RenderSettings(bpy.types.PropertyGroup):
    def nodetree_update(self, context):
        if not self.data_source:
            self.nodetree_camera = ""
            return

        output_node = bpy.data.node_groups[self.data_source].output_node
        if not output_node:
            self.nodetree_camera = ""
            return

        # stage = output_node.cached_stage()
        # if not stage:
        #     self.nodetree_camera = ""
        #     return
        #
        # if self.nodetree_camera:
        #     prim = stage.GetPrimAtPath(self.nodetree_camera)
        #     if prim and prim.GetTypeName() == "Camera":
        #         return
        #
        # self.nodetree_camera = ""
        # for prim in stage.TraverseAll():
        #     if prim.GetTypeName() == "Camera":
        #         self.nodetree_camera = prim.GetPath().pathString
        #         break


class FinalRenderSettings(RenderSettings):
    data_source: bpy.props.StringProperty(
        name="Data Source",
        description="Data source for final render",
        default=""
    )
    nodetree_camera: bpy.props.StringProperty(
        name="Camera",
        description="Select camera from USD for final render",
        default=""
    )


class ViewportRenderSettings(RenderSettings):
    def data_source_update(self, context):
        usd_collection.update(context)
    #
    # def nodetree_camera_update(self, context):
    #     viewport_camera = context.scene.objects.get(USD_CAMERA, None)
    #     if not self.data_source:
    #         if viewport_camera:
    #             bpy.data.objects.remove(viewport_camera)
    #         return
    #
    #     output_node = bpy.data.node_groups[self.data_source].output_node
    #     if not output_node:
    #         return
    #
    #     stage = output_node.cached_stage()
    #     if not stage:
    #         return
    #
    #     camera_prim = stage.GetPrimAtPath(self.nodetree_camera)
    #     if not camera_prim:
    #         if viewport_camera:
    #             bpy.data.objects.remove(viewport_camera)
    #         return
    #
    #     camera_settings = CameraData.init_from_usd_camera(camera_prim)
    #     if not viewport_camera:
    #         camera_data = bpy.data.cameras.new(USD_CAMERA)
    #         viewport_camera = bpy.data.objects.new(USD_CAMERA, camera_data)
    #         context.scene.collection.objects.link(viewport_camera)
    #         context.scene.camera = viewport_camera
    #
    #     camera_settings.export_to_camera(viewport_camera)

    data_source: bpy.props.StringProperty(
        name="Data Source",
        description="Data source for viewport render",
        default="",
        update=data_source_update
    )
    nodetree_camera: bpy.props.StringProperty(
        name="Camera",
        description="Select camera from USD for viewport render",
        default="",
        #update=nodetree_camera_update
    )


class SceneProperties(USDHydraProperties):
    bl_type = bpy.types.Scene

    final: bpy.props.PointerProperty(type=FinalRenderSettings)
    viewport: bpy.props.PointerProperty(type=ViewportRenderSettings)
