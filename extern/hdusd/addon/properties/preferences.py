# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import tempfile
import _hdusd
from logging import getLevelName
from pathlib import Path
from bpy.types import AddonPreferences
from bpy.props import StringProperty, BoolProperty, EnumProperty
from ..utils import logging

log = logging.Log('preferences')


class HDUSD_ADDON_PT_preferences(AddonPreferences):
    bl_idname = "hdusd"

    def update_temp_dir(self, value):
        if not Path(self.tmp_dir).exists() or tempfile.gettempdir() == str(Path(self.tmp_dir)):
            log.info(f"Current temp directory is {tempfile.gettempdir()}")
            return

        tempfile.tempdir = Path(self.tmp_dir)
        bpy.context.preferences.addons['hdusd'].preferences['tmp_dir'] = str(_hdusd.utils.get_temp_dir())
        log.info(f"Current temp directory is changed to {bpy.context.preferences.addons['hdusd'].preferences.tmp_dir}")

    def update_log_level(self, context):
        logging.logger.setLevel(self.log_level)
        log.critical(f"Log level is set to {self.log_level}")

    tmp_dir: StringProperty(
        name="Temp Directory",
        description="Set temp directory",
        maxlen=1024,
        subtype='DIR_PATH',
        default=str(_hdusd.utils.get_temp_dir()),
        update=update_temp_dir,
    )
    dev_tools: BoolProperty(
        name="Developer Tools",
        description="Enable developer tools",
        default=False,
    )
    log_level: EnumProperty(
        name="Log Level",
        description="Select logging level",
        items=(('DEBUG', "Debug", "Log level DEBUG"),
               ('INFO', "Info", "Log level INFO"),
               ('WARNING', "Warning", "Log level WARN"),
               ('ERROR', "Error", "Log level ERROR"),
               ('CRITICAL', "Critical", "Log level CRITICAL")),
        default=getLevelName(logging.logger.level),
        update=update_log_level,

    )
    def draw(self, context):
        layout = self.layout
        col = layout.column()
        col.prop(self, "tmp_dir", icon='NONE' if Path(self.tmp_dir).exists() else 'ERROR')
        col.prop(self, "dev_tools")
        col.prop(self, "log_level")
        col.separator()
        #row = col.row()
        #row.operator("wm.url_open", text="Main Site", icon='URL').url = bl_info["main_web"]
        #row.operator("wm.url_open", text="Community", icon='COMMUNITY').url = bl_info["community"]


def get_addon_pref():
    return bpy.context.preferences.addons['hdusd'].preferences
