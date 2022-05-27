# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import bpy
import nodeitems_utils
from nodeitems_utils import NodeCategory, NodeItem
# from .. import log

# classes to register
from . import (
    input, output, converter, transformations
)


class USDNodeCategory(NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'hdusd.USDTree'


node_categories = [
    USDNodeCategory('HdUSD_USD_INPUT', "Input", items=[
        # NodeItem('hdusd.BlenderDataNode'),
        NodeItem('hdusd.UsdFileNode'),
    ]),
    USDNodeCategory('HdUSD_USD_OUTPUT', 'Output', items=[
        NodeItem('hdusd.OutputNode'),
    ]),
    USDNodeCategory('HdUSD_USD_CONVERTER', 'Converter', items=[
        NodeItem('hdusd.MergeNode'),
        NodeItem('hdusd.FilterNode'),
        NodeItem('hdusd.RootNode'),
        # NodeItem('hdusd.InstancingNode'),
    ]),
    USDNodeCategory('HdUSD_USD_TRANSFORMATIONS', 'Transformations', items=[
        NodeItem('hdusd.TransformNode'),
        NodeItem('hdusd.TransformByEmptyNode'),
    ]),
    USDNodeCategory('HdUSD_USD_LAYOUT', 'Layout', items=[
        NodeItem('NodeFrame'),
        NodeItem('NodeReroute'),
    ]),
]

# nodes to register
register_classes, unregister_classes = bpy.utils.register_classes_factory([
    input.UsdFileNode,

    output.OutputNode,

    converter.MergeNode,
    converter.FilterNode,
    converter.RootNode,

    transformations.HDUSD_USD_NODETREE_OP_transform_add_empty,
    transformations.TransformNode,
    transformations.TransformByEmptyNode,
])


def register():
    register_classes()
    nodeitems_utils.register_node_categories("USD_NODES", node_categories)


def unregister():
    nodeitems_utils.unregister_node_categories("USD_NODES")
    unregister_classes()
