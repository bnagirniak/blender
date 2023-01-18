# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

def export(material_name, file_path):
    import bpy

    from . import logger
    log = logger.Log('matx')

    try:
        import materialx.utils as mx_utils

    except ImportError as e:
        log.error("No MaterialX addon")
        return False

    material = bpy.data.materials[material_name]
    doc = mx_utils.export(material, None)
    if not doc:
        return False

    mx_utils.export_to_file(doc, file_path, False)
    return True
