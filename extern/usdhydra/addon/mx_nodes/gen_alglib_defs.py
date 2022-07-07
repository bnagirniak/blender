# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>


import bpy
from bpy.props import (
    EnumProperty,
    FloatProperty,
    IntProperty,
    BoolProperty,
    StringProperty,
    PointerProperty,
    FloatVectorProperty,
)
from .base_node import MxNode


FILE_PATH = r"libraries\alglib\alglib_defs.mtlx"


class MxNode_ALG_algclamp(MxNode):
    _file_path = FILE_PATH
    _data_types = {'float': {'nd_name': 'ND_algclamp_float', 'nd': None}, 'vector2': {'nd_name': 'ND_algclamp_vector2', 'nd': None}, 'vector3': {'nd_name': 'ND_algclamp_vector3', 'nd': None}, 'vector4': {'nd_name': 'ND_algclamp_vector4', 'nd': None}, 'color3': {'nd_name': 'ND_algclamp_color3', 'nd': None}}

    bl_label = 'Algclamp'
    bl_idname = 'usdhydra.MxNode_ALG_algclamp'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('float', 'Float', 'Float'), ('vector2', 'Vector2', 'Vector2'), ('vector3', 'Vector3', 'Vector3'), ('vector4', 'Vector4', 'Vector4'), ('color3', 'Color3', 'Color3')], default='color3', update=MxNode.update_data_type)

    nd_float_in_val: FloatProperty(name="Value", description="", default=0.0, update=MxNode.update_prop)
    nd_float_in_i_min: FloatProperty(name="Min", description="", default=0.0, update=MxNode.update_prop)
    nd_float_in_i_max: FloatProperty(name="Max", description="", default=1.0, update=MxNode.update_prop)
    nd_float_out_out: FloatProperty(name="Out", description="", update=MxNode.update_prop)

    nd_vector2_in_val: FloatVectorProperty(name="Value", description="", subtype="NONE", size=2, default=(0.0, 0.0), update=MxNode.update_prop)
    nd_vector2_in_i_min: FloatVectorProperty(name="Min", description="", subtype="NONE", size=2, default=(0.0, 0.0), update=MxNode.update_prop)
    nd_vector2_in_i_max: FloatVectorProperty(name="Max", description="", subtype="NONE", size=2, default=(1.0, 1.0), update=MxNode.update_prop)
    nd_vector2_out_out: FloatVectorProperty(name="Out", description="", subtype="NONE", size=2, update=MxNode.update_prop)

    nd_vector3_in_val: FloatVectorProperty(name="Value", description="", subtype="XYZ", size=3, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector3_in_i_min: FloatVectorProperty(name="Min", description="", subtype="XYZ", size=3, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector3_in_i_max: FloatVectorProperty(name="Max", description="", subtype="XYZ", size=3, default=(1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_vector3_out_out: FloatVectorProperty(name="Out", description="", subtype="XYZ", size=3, update=MxNode.update_prop)

    nd_vector4_in_val: FloatVectorProperty(name="Value", description="", subtype="NONE", size=4, default=(0.0, 0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector4_in_i_min: FloatVectorProperty(name="Min", description="", subtype="NONE", size=4, default=(0.0, 0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector4_in_i_max: FloatVectorProperty(name="Max", description="", subtype="NONE", size=4, default=(1.0, 1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_vector4_out_out: FloatVectorProperty(name="Out", description="", subtype="NONE", size=4, update=MxNode.update_prop)

    nd_color3_in_val: FloatVectorProperty(name="Value", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_in_i_min: FloatVectorProperty(name="Min", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_in_i_max: FloatVectorProperty(name="Max", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_color3_out_out: FloatVectorProperty(name="Out", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, update=MxNode.update_prop)


class MxNode_ALG_alglevels(MxNode):
    _file_path = FILE_PATH
    _data_types = {'float': {'nd_name': 'ND_alglevels_float', 'nd': None}, 'vector2': {'nd_name': 'ND_alglevels_vector2', 'nd': None}, 'vector3': {'nd_name': 'ND_alglevels_vector3', 'nd': None}, 'vector4': {'nd_name': 'ND_alglevels_vector4', 'nd': None}, 'color3': {'nd_name': 'ND_alglevels_color3', 'nd': None}}

    bl_label = 'Alglevels'
    bl_idname = 'usdhydra.MxNode_ALG_alglevels'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('float', 'Float', 'Float'), ('vector2', 'Vector2', 'Vector2'), ('vector3', 'Vector3', 'Vector3'), ('vector4', 'Vector4', 'Vector4'), ('color3', 'Color3', 'Color3')], default='color3', update=MxNode.update_data_type)

    nd_float_in_pixel: FloatProperty(name="Value", description="", default=0.0, update=MxNode.update_prop)
    nd_float_in_i_min: FloatProperty(name="Min", description="", default=0.0, update=MxNode.update_prop)
    nd_float_in_i_max: FloatProperty(name="Max", description="", default=1.0, update=MxNode.update_prop)
    nd_float_in_i_gamma: FloatProperty(name="Gamma", description="", default=1.0, update=MxNode.update_prop)
    nd_float_out_out: FloatProperty(name="Out", description="", update=MxNode.update_prop)

    nd_vector2_in_pixel: FloatVectorProperty(name="Value", description="", subtype="NONE", size=2, default=(0.0, 0.0), update=MxNode.update_prop)
    nd_vector2_in_i_min: FloatVectorProperty(name="Min", description="", subtype="NONE", size=2, default=(0.0, 0.0), update=MxNode.update_prop)
    nd_vector2_in_i_max: FloatVectorProperty(name="Max", description="", subtype="NONE", size=2, default=(1.0, 1.0), update=MxNode.update_prop)
    nd_vector2_in_i_gamma: FloatProperty(name="Gamma", description="", default=1.0, update=MxNode.update_prop)
    nd_vector2_out_out: FloatVectorProperty(name="Out", description="", subtype="NONE", size=2, update=MxNode.update_prop)

    nd_vector3_in_pixel: FloatVectorProperty(name="Value", description="", subtype="XYZ", size=3, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector3_in_i_min: FloatVectorProperty(name="Min", description="", subtype="XYZ", size=3, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector3_in_i_max: FloatVectorProperty(name="Max", description="", subtype="XYZ", size=3, default=(1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_vector3_in_i_gamma: FloatProperty(name="Gamma", description="", default=1.0, update=MxNode.update_prop)
    nd_vector3_out_out: FloatVectorProperty(name="Out", description="", subtype="XYZ", size=3, update=MxNode.update_prop)

    nd_vector4_in_pixel: FloatVectorProperty(name="Value", description="", subtype="NONE", size=4, default=(0.0, 0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector4_in_i_min: FloatVectorProperty(name="Min", description="", subtype="NONE", size=4, default=(0.0, 0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_vector4_in_i_max: FloatVectorProperty(name="Max", description="", subtype="NONE", size=4, default=(1.0, 1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_vector4_in_i_gamma: FloatProperty(name="Gamma", description="", default=1.0, update=MxNode.update_prop)
    nd_vector4_out_out: FloatVectorProperty(name="Out", description="", subtype="NONE", size=4, update=MxNode.update_prop)

    nd_color3_in_pixel: FloatVectorProperty(name="Value", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_in_i_min: FloatVectorProperty(name="Min", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_in_i_max: FloatVectorProperty(name="Max", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(1.0, 1.0, 1.0), update=MxNode.update_prop)
    nd_color3_in_i_gamma: FloatProperty(name="Gamma", description="", default=1.0, update=MxNode.update_prop)
    nd_color3_out_out: FloatVectorProperty(name="Out", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, update=MxNode.update_prop)


class MxNode_ALG_algdot(MxNode):
    _file_path = FILE_PATH
    _data_types = {'color3': {'nd_name': 'ND_algdot_color3', 'nd': None}}

    bl_label = 'Algdot'
    bl_idname = 'usdhydra.MxNode_ALG_algdot'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('color3', 'Color3', 'Color3')], default='color3', update=MxNode.update_data_type)

    nd_color3_in_in1: FloatVectorProperty(name="Value", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_in_in2: FloatVectorProperty(name="Min", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_out_out: FloatProperty(name="Out", description="", update=MxNode.update_prop)


class MxNode_ALG_algheighttonormal(MxNode):
    _file_path = FILE_PATH
    _data_types = {'vector3': {'nd_name': 'ND_algheighttonormal', 'nd': None}}

    bl_label = 'Algheighttonormal'
    bl_idname = 'usdhydra.MxNode_ALG_algheighttonormal'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('vector3', 'Vector3', 'Vector3')], default='vector3', update=MxNode.update_data_type)

    nd_vector3_in_h: FloatProperty(name="Height", description="", default=0.0, update=MxNode.update_prop)
    nd_vector3_in_dU: FloatProperty(name="HeightU", description="", default=0.0, update=MxNode.update_prop)
    nd_vector3_in_dV: FloatProperty(name="HeightV", description="", default=0.0, update=MxNode.update_prop)
    nd_vector3_in_delta: FloatProperty(name="Delta", description="", default=0.0001, update=MxNode.update_prop)
    nd_vector3_in_intensity: FloatProperty(name="Normal Intensity", description="", min=0.001, max=10.0, default=1.0, update=MxNode.update_prop)
    nd_vector3_out_out: FloatVectorProperty(name="Out", description="", subtype="XYZ", size=3, update=MxNode.update_prop)


class MxNode_ALG_algnormalTStoWS(MxNode):
    _file_path = FILE_PATH
    _data_types = {'vector3': {'nd_name': 'ND_algnormalTStoWS_vector3', 'nd': None}}

    bl_label = 'AlgnormalTStoWS'
    bl_idname = 'usdhydra.MxNode_ALG_algnormalTStoWS'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('vector3', 'Vector3', 'Vector3')], default='vector3', update=MxNode.update_data_type)

    nd_vector3_in_normalTS: FloatVectorProperty(name="Normal Tangent Space", description="", subtype="XYZ", size=3, default=(0.5, 0.5, 1.0), update=MxNode.update_prop)
    nd_vector3_in_openGL: BoolProperty(name="OpenGL Tangent Space", description="", default=True, update=MxNode.update_prop)
    nd_vector3_in_index: IntProperty(name="UV Index", description="", default=0, update=MxNode.update_prop)
    nd_vector3_out_out: FloatVectorProperty(name="Out", description="", subtype="XYZ", size=3, update=MxNode.update_prop)


class MxNode_ALG_algsrgb_to_linear(MxNode):
    _file_path = FILE_PATH
    _data_types = {'color3': {'nd_name': 'ND_algsrgb_to_linear', 'nd': None}}

    bl_label = 'Algsrgb to linear'
    bl_idname = 'usdhydra.MxNode_ALG_algsrgb_to_linear'
    bl_description = ""

    category = 'Algorithm'

    data_type: EnumProperty(name="Type", description="Input Data Type", items=[('color3', 'Color3', 'Color3')], default='color3', update=MxNode.update_data_type)

    nd_color3_in_in: FloatVectorProperty(name="In", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, default=(0.0, 0.0, 0.0), update=MxNode.update_prop)
    nd_color3_out_out: FloatVectorProperty(name="Out", description="", subtype="COLOR", size=3, soft_min=0.0, soft_max=1.0, update=MxNode.update_prop)


mx_node_classes = [MxNode_ALG_algclamp, MxNode_ALG_alglevels, MxNode_ALG_algdot, MxNode_ALG_algheighttonormal, MxNode_ALG_algnormalTStoWS, MxNode_ALG_algsrgb_to_linear]
