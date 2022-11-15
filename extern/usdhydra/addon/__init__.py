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

import _usdhydra

from . import logger
log = logger.Log('init')

from . import preferences, engine


def register():
    log("register")

    preferences.register()
    engine.register()

    _usdhydra.init()


def unregister():
    log("unregister")

    _usdhydra.exit()

    engine.unregister()
    preferences.unregister()
