# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import logger
log = logger.Log('matx')


def export(material_name, file_path):
    try:
        import materialx.utils as mx_utils

        material = bpy.data.materials[material_name]
        doc = mx_utils.export(material, None)
        if not doc:
            return False

        mx_utils.export_to_file(doc, file_path, False)
        return True

    except Exception as e:
        log.error(e)

    return False
