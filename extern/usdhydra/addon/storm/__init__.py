# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import engine, properties, ui


register, unregister = bpy.utils.register_classes_factory((
    engine.StormHydraRenderEngine,
    properties.SceneProperties,
    ui.STORM_HYDRA_RENDER_PT_render_settings,
))
