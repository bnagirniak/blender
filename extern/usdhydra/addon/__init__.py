# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

bl_info = {
    "name": "USD Hydra render engine",
    "author": "AMD",
    "version": (1, 0, 0),
    "blender": (3, 2, 0),
    "location": "Info header > Render engine menu",
    "description": "USD Hydra renderer integration",
    "tracker_url": "",
    "doc_url": "",
    "community": "",
    "downloads": "",
    "main_web": "",
    "support": 'TESTING',
    "category": "Render"
}

import atexit

import bpy
from bpy.utils import register_class, unregister_class
import _usdhydra

from .utils import logging
log = logging.Log('init')

from . import (
    properties,
    engine,
    usd_nodes,
    ui,
    handlers,
    preferences,
    storm_engine,
)


def exit():
    engine.exit()


def register():
    # Make sure we only registered the callback once.
    atexit.unregister(exit)
    atexit.register(exit)

    register_class(preferences.AddonPreferences)
    preferences.addon_preferences().init()

    properties.register()
    ui.register()
    usd_nodes.register()
    handlers.register()
    storm_engine.register()

    from .utils import enable_delegates
    enable_delegates()

    _usdhydra.init()


def unregister():
    from .utils import disable_delegates
    disable_delegates()

    ui.unregister()
    properties.unregister()
    usd_nodes.unregister()
    handlers.unregister()
    storm_engine.unregister()

    unregister_class(preferences.AddonPreferences)
