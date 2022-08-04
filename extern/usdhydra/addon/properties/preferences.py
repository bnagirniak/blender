# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import tempfile
from logging import getLevelName
from pathlib import Path

import bpy
from bpy.types import AddonPreferences, Operator
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy_extras.io_utils import ImportHelper

import _usdhydra
from ..utils import logging
from ..utils.delegate import manager

log = logging.Log('preferences')


DELEGATES_DIR = str(bpy.utils.system_resource('SCRIPTS', path="addons") + "/usdhydra/delegates")


class USDHYDRA_ADDON_OP_install_delegate(Operator, ImportHelper):
    bl_idname = "usdhydra.install_render_delegate"
    bl_label = "Add Render Delegate"

    filename_ext = ".zip"
    filepath: bpy.props.StringProperty(
        name="File Path",
        maxlen=1024, subtype="FILE_PATH"
    )
    filter_glob: bpy.props.StringProperty(default="*.zip")

    @classmethod
    def poll(cls, context):
        return not manager.in_progress

    def execute(self, context):
        manager.filepath = self.filepath
        manager.install_delegate()
        return {'FINISHED'}


class USDHYDRA_ADDON_PT_preferences(AddonPreferences):
    bl_idname = "usdhydra"

    def update_temp_dir(self, value):
        if not Path(self.tmp_dir).exists() or tempfile.gettempdir() == str(Path(self.tmp_dir)):
            log.info(f"Current temp directory is {tempfile.gettempdir()}")
            return

        tempfile.tempdir = Path(self.tmp_dir)
        bpy.context.preferences.addons['usdhydra'].preferences['tmp_dir'] = str(_usdhydra.utils.get_temp_dir())
        log.info(f"Current temp directory is changed to {bpy.context.preferences.addons['usdhydra'].preferences.tmp_dir}")

    def update_log_level(self, context):
        logging.logger.setLevel(self.log_level)
        log.critical(f"Log level is set to {self.log_level}")

    tmp_dir: StringProperty(
        name="Temp Directory",
        description="Set temp directory",
        maxlen=1024,
        subtype='DIR_PATH',
        default=_usdhydra.utils.get_temp_dir(),
        update=update_temp_dir,
    )
    delegates_dir: StringProperty(
        name="Delegate Directory",
        description="Set delegate directory",
        maxlen=1024,
        subtype='DIR_PATH',
        default=DELEGATES_DIR,
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
    show_settings: BoolProperty(
        name="Developer Settings",
        default=False,
    )
    show_delegate: BoolProperty(
        name="Render Delegate",
        default=False,
    )
    settings: EnumProperty(
        name="",
        items=(('SETTINGS', "Settings", "Developer settings"),
               ('DELEGATE', "Delegates", "Render delegates settings")),
        default='SETTINGS',
    )

    def draw(self, context):
        def _draw_settings(layout):
            layout.separator()
            layout.prop(self, "tmp_dir", icon='NONE' if Path(self.tmp_dir).exists() else 'ERROR')
            layout.prop(self, "dev_tools")
            layout.prop(self, "log_level")
            layout.separator()

        def _draw_delegates(layout):
            row = layout.row()
            split = row.split(factor=0.8)
            split.prop(self, "delegates_dir")
            split.operator(USDHYDRA_ADDON_OP_install_delegate.bl_idname, icon='IMPORT', text="Install")

            if manager.delegates is None:
                manager.update_delegates()

            layout.separator()
            for key, value, path, *_ in manager.delegates:
                row = layout.box().row(align=True)
                row.alignment = 'LEFT'
                split = row.split(factor=0.5)
                split1 = split.split(factor=0.7)
                split1.label(text=str(key))
                split1.label(text=str(value))
                split.label(text=str(path))

            layout.separator()

        layout = self.layout
        col = layout.column()
        row = col.row(align=True)
        row.prop(self, 'settings', expand=True, text=" ")

        layout = layout.column()
        if self.settings == 'SETTINGS':
            _draw_settings(layout)

        if self.settings == 'DELEGATE':
            _draw_delegates(layout)

        # row = col.row()
        #row.operator("wm.url_open", text="Main Site", icon='URL').url = bl_info["main_web"]
        #row.operator("wm.url_open", text="Community", icon='COMMUNITY').url = bl_info["community"]


def get_addon_pref():
    return bpy.context.preferences.addons['usdhydra'].preferences
