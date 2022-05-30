# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import _hdusd


_stages = {}


def key(bl_obj):
    return bl_obj.as_pointer()


def get(bl_obj):
    return _stages.get(key(bl_obj), 0)


def set(bl_obj, stage):
    free(bl_obj)
    _stages[key(bl_obj)] = stage


def free(bl_obj):
    if key(bl_obj) not in _stages:
        return

    stage = _stages.pop(key(bl_obj))
    if stage not in _stages.values():
        _hdusd.stage_free(stage)


def free_all():
    for stage in _stages.values():
        _hdusd.stage_free(stage)

    _stages.clear()
