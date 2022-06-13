# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>


bl_info = {
    "name": "USD Hydra render engine",
    "author": "AMD",
    "version": (2, 0, 0),
    "blender": (3, 1, 0),
    "location": "Info header > Render engine menu",
    "description": "USD Hydra renderer integration",
    "tracker_url": "https://github.com/GPUOpen-LibrariesAndSDKs/BlenderUSDHydraAddon/issues",
    "doc_url": "https://radeon-pro.github.io/RadeonProRenderDocs/en/usd_hydra/about.html",
    "community": "https://github.com/GPUOpen-LibrariesAndSDKs/BlenderUSDHydraAddon/discussions",
    "downloads": "https://github.com/GPUOpen-LibrariesAndSDKs/BlenderUSDHydraAddon/releases",
    "main_web": "https://www.amd.com/en/technologies/radeon-prorender",
    "support": 'TESTING',
    "category": "Render"
}

# Support 'reload' case.
if "bpy" in locals():
    import importlib
    if "engine" in locals():
        importlib.reload(engine)
    if "ui" in locals():
        importlib.reload(ui)
    # if "operators" in locals():
    #     importlib.reload(operators)
    if "properties" in locals():
        importlib.reload(properties)


from . import (
    engine,
    usd_nodes,
    properties,
    ui,
    handlers,
)

import bpy
import tempfile
import _hdusd
from logging import getLevelName
from pathlib import Path
from bpy.types import AddonPreferences
from bpy.props import StringProperty, BoolProperty, EnumProperty
from .utils import logging

log = logging.Log('init')


class HDUSD_ADDON_PT_preferences(AddonPreferences):
    bl_idname = __name__

    def update_temp_dir(self, value):
        if not Path(self.tmp_dir).exists() or tempfile.gettempdir() == str(Path(self.tmp_dir)):
            log.info(f"Current temp directory is {tempfile.gettempdir()}")
            return

        tempfile.tempdir = Path(self.tmp_dir)
        bpy.context.preferences.addons['hdusd'].preferences['tmp_dir'] = str(_hdusd.get_temp_dir())
        log.info(f"Current temp directory is changed to {bpy.context.preferences.addons['hdusd'].preferences.tmp_dir}")

    def update_log_level(self, context):
        logging.logger.setLevel(self.log_level)
        log.critical(f"Log level is set to {self.log_level}")

    tmp_dir: StringProperty(
        name="Temp Directory",
        description="Set temp directory",
        maxlen=1024,
        subtype='DIR_PATH',
        default=str(_hdusd.get_temp_dir()),
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
        row = col.row()
        row.operator("wm.url_open", text="Main Site", icon='URL').url = bl_info["main_web"]
        row.operator("wm.url_open", text="Community", icon='COMMUNITY').url = bl_info["community"]


def exit():
    engine.exit()


def register():
    import atexit
    from bpy.utils import register_class
    # from . import ui
    # from . import operators
    # from . import properties
    # from . import presets

    # Make sure we only registered the callback once.
    atexit.unregister(exit)
    atexit.register(exit)

    engine.init()

    properties.register()
    ui.register()
    # operators.register()
    # presets.register()
    usd_nodes.register()
    handlers.register()
    bpy.utils.register_class(HDUSD_ADDON_PT_preferences)

    register_class(engine.HdUSDEngine)


def unregister():
    from bpy.utils import unregister_class
    # from . import ui
    # from . import operators
    # from . import properties
    # from . import presets

    # bpy.app.handlers.version_update.remove(version_update.do_versions)

    ui.unregister()
    # operators.unregister()
    properties.unregister()
    # presets.unregister()
    usd_nodes.unregister()
    handlers.unregister()
    bpy.utils.unregister_class(HDUSD_ADDON_PT_preferences)

    unregister_class(engine.HdUSDEngine)
