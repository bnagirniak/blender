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

# Support 'reload' case.
if "bpy" in locals():
    import importlib
    importlib.reload(properties)
    importlib.reload(engine)
    importlib.reload(ui)
    # importlib.reload(operators)

else:
    from . import (
        mx_nodes,
        properties,
        engine,
        usd_nodes,
        ui,
        handlers,
    )

import atexit
import bpy
from bpy.utils import register_class, unregister_class
from .utils import logging

log = logging.Log('init')


def create_delegates_folder():
    from pathlib import Path
    from .properties import preferences
    Path(preferences.DELEGATES_DIR).mkdir(exist_ok=True)
    log(preferences.DELEGATES_DIR)


def exit():
    engine.exit()


def register():
    create_delegates_folder()
    # Make sure we only registered the callback once.
    atexit.unregister(exit)
    atexit.register(exit)

    mx_nodes.register()
    properties.register()

    engine.init()
    ui.register()
    # operators.register()
    # presets.register()
    usd_nodes.register()
    handlers.register()

    register_class(engine.USDHydraEngine)


def unregister():
    ui.unregister()
    # operators.unregister()
    mx_nodes.unregister()
    properties.unregister()
    # presets.unregister()
    usd_nodes.unregister()
    handlers.unregister()

    unregister_class(engine.USDHydraEngine)
