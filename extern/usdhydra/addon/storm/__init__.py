# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

from . import engine, properties, ui


def register():
    properties.register()
    ui.register()
    engine.register()


def unregister():
    properties.unregister()
    ui.unregister()
    engine.unregister()
