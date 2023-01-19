# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import traceback

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

        mtlx_file = mx_utils.get_temp_file(".mtlx", material.name, True)
        mx_utils.export_to_file(doc, mtlx_file, False)
        return str(mtlx_file)

    except Exception as e:
        log.error(e, 'EXCEPTION:', traceback.format_exc())

    return ""
