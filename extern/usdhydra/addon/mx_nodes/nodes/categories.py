# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

from collections import defaultdict

from nodeitems_utils import NodeCategory, NodeItem

from ...utils import title_str, code_str


class MxNodeCategory(NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'usdhydra.MxNodeTree'


def get_node_categories():
    from . import mx_node_classes

    d = defaultdict(list)
    for MxNode_cls in mx_node_classes:
        d[MxNode_cls.category].append(MxNode_cls)

    categories = []
    for category, category_classes in d.items():
        categories.append(
            MxNodeCategory('USDHYDRA_MX_NG_' + code_str(category), title_str(category),
                           items=[NodeItem(MxNode_cls.bl_idname)
                                  for MxNode_cls in category_classes]))

    categories.append(
        MxNodeCategory('USDHYDRA_MX_NG_LAYOUT', 'Layout',
                       items=[NodeItem("NodeFrame"),
                              NodeItem("NodeReroute")]))

    return categories
