# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy

from . import logger
log = logger.Log('matx')


def export(material_name):
    try:
        import materialx.utils as mx_utils

        material = bpy.data.materials[material_name]
        doc = mx_utils.export(material, None)
        if not doc:
            return ""

        mtlx_file = mx_utils.get_temp_file(
            ".mtlx", f'{material.name}_{material.node_tree.name if material.node_tree else ""}')
        mx_utils.export_to_file(doc, mtlx_file, False)
        return str(mtlx_file)

    except Exception as e:
        log.error(e)

    return ""
