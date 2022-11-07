# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
from bpy.utils import register_class, unregister_class
import _usdhydra

from .utils import logging
log = logging.Log('preferences')


class AddonPreferences(bpy.types.AddonPreferences):
    bl_idname = "usdhydra"

    def init(self):
        self.update_log_level(None)

    def update_log_level(self, context):
        logging.logger.setLevel(self.log_level)

    def update_storm_delegate(self, context):
        from .storm_engine import USDHydraHdStormEngine

        if self.storm_delegate:
            register_class(USDHydraHdStormEngine)
        else:
            unregister_class(USDHydraHdStormEngine)

    dev_tools: bpy.props.BoolProperty(
        name="Developer Tools",
        description="Enable developer tools",
        default=False,
    )
    log_level: bpy.props.EnumProperty(
        name="Log Level",
        description="Select logging level",
        items=(('DEBUG', "Debug", "Log level DEBUG"),
               ('INFO', "Info", "Log level INFO"),
               ('WARNING', "Warning", "Log level WARN"),
               ('ERROR', "Error", "Log level ERROR"),
               ('CRITICAL', "Critical", "Log level CRITICAL")),
        default='INFO',
        update=update_log_level,
    )
    storm_delegate: bpy.props.BoolProperty(
        name="Storm Render Delegate",
        description="Enable Hydra Storm (OpenGL) render delegate",
        default=True,
        update=update_storm_delegate,
    )

    def draw(self, context):
        layout = self.layout
        box = layout.box()
        box.prop(self, 'storm_delegate')
        box = layout.box()
        box.prop(self, 'dev_tools')
        box.prop(self, 'log_level')


def addon_preferences():
    return bpy.context.preferences.addons['usdhydra'].preferences
