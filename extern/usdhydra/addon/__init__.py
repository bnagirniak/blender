# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

bl_info = {
    "name": "USD Hydra render engine",
    "author": "AMD",
    "version": (1, 0, 0),
    "blender": (3, 5, 0),
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
import _usdhydra

import logger
log = logger.Log('init')

from . import (
    engine,
    preferences,
)
from storm import engine


def exit():
    _usdhydra.exit()


def register():
    # Make sure we only registered the callback once.
    atexit.unregister(exit)
    atexit.register(exit)

    bpy.utils.register_class(preferences.AddonPreferences)
    preferences.addon_preferences().init()

    engine.register()

    _usdhydra.init()


def unregister():
    exit()

    engine.unregister()

    bpy.utils.unregister_class(preferences.AddonPreferences)
