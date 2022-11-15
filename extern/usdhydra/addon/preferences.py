# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import logger
log = logger.Log('preferences')


class AddonPreferences(bpy.types.AddonPreferences):
    bl_idname = "usdhydra"

    def init(self):
        self.update_log_level(None)

    def update_log_level(self, context):
        log("update_log_level", self.log_level)
        logger.logger.setLevel(self.log_level)

    def update_storm_render_engine(self, context):
        from .storm.engine import HdStormHydraRenderEngine

        log("update_storm_render_engine", self.storm_render_engine)
        if self.storm_render_engine:
            bpy.utils.register_class(HdStormHydraRenderEngine)
        else:
            bpy.utils.unregister_class(HdStormHydraRenderEngine)

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
    storm_render_engine: bpy.props.BoolProperty(
        name="Storm Render Engine",
        description="Enable Hydra Storm (OpenGL) render engine delegate",
        default=True,
        update=update_storm_render_engine,
    )

    def draw(self, context):
        layout = self.layout

        box = layout.box()
        box.prop(self, 'storm_render_engine')

        box = layout.box()
        # box.prop(self, 'dev_tools')
        box.prop(self, 'log_level')


def addon_preferences():
    return bpy.context.preferences.addons['usdhydra'].preferences
