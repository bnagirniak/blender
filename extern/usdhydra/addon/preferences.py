# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import _usdhydra

from .utils import logging
log = logging.Log('preferences')


class AddonPreferences(bpy.types.AddonPreferences):
    bl_idname = "usdhydra"

    def init(self):
        self.update_log_level(None)

    def update_log_level(self, context):
        logging.logger.setLevel(self.log_level)

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
    )

    def draw(self, context):
        layout = self.layout
        layout.separator()
        # layout.prop(self, "tmp_dir", icon='NONE' if Path(self.tmp_dir).exists() else 'ERROR')
        layout.prop(self, "dev_tools")
        layout.prop(self, "log_level")
        layout.separator()



        # row = col.row()
        #row.operator("wm.url_open", text="Main Site", icon='URL').url = bl_info["main_web"]
        #row.operator("wm.url_open", text="Community", icon='COMMUNITY').url = bl_info["community"]


def addon_preferences():
    return bpy.context.preferences.addons['usdhydra'].preferences
