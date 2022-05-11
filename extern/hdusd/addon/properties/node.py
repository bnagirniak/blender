# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
from . import HdUSDProperties


class NodeProperties(HdUSDProperties):
    bl_type = bpy.types.Node

    stage: bpy.props.IntProperty()
