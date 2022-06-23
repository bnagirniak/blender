# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import HdUSDProperties
from .usd_stage import UsdStage


class WindowManagerProperties(HdUSDProperties):
    bl_type = bpy.types.WindowManager

    usd_stages: bpy.props.CollectionProperty(type=UsdStage)
