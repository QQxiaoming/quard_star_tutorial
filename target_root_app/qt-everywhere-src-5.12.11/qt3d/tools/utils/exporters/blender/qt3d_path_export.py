#############################################################################
##
## Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt3D module of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

# Required Blender information.
bl_info = {
           "name": "Qt3D Path Exporter",
           "author": "Sean Harmer <sean.harmer@kdab.com>, Robert Brock <robert.brock@kdab.com>",
           "version": (0, 1),
           "blender": (2, 78, 0),
           "location": "File > Export > Qt3D Path Exporter (.json)",
           "description": "Export path to json to use with Qt3D",
           "warning": "",
           "wiki_url": "",
           "tracker_url": "",
           "category": "Import-Export"
          }

import bpy
import os
import struct
import mathutils
import math
import json
from array import array
from bpy_extras.io_utils import ExportHelper
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from collections import defaultdict

def jsonBuilder():
    pathList = []
    name = ""

    obj = bpy.context.object
    curve = obj.data
    spline = curve.splines.active

    for point in spline.points:
            # add the stuff
            pathList.append({"position": str("%.0f, %.0f, %.0f" % (point.co[0] * -1,
                                                                   point.co[2],
                                                                   point.co[1]))})

    return pathList

class Qt3DPathDataConverter:

    def pathInfoToJson(self):
        # Pass 1 - collect data we need to produce the output in pass 2
        print(">>> About to enter Objects")

        jsonData = json.dumps({ "splines": jsonBuilder()}, indent=2, sort_keys=True, separators=(',', ': '))
        return jsonData


class Qt3DTDPathExporter(bpy.types.Operator, ExportHelper):
    """Qt3D Exporter"""
    bl_idname       = "export_scene.qt3d_td_path_exporter";
    bl_label        = "Qt3DPathExporter";
    bl_options      = {'PRESET'};

    filename_ext = ""
    use_filter_folder = True

    def __init__(self):
        pass

    def execute(self, context):
        print("In Execute" + bpy.context.scene.name)

        self.userpath = self.properties.filepath

        # unselect all
        bpy.ops.object.select_all(action='DESELECT')

        converter = Qt3DPathDataConverter()
        fileContent = converter.pathInfoToJson()
        with open(self.userpath + ".json", '+w') as f:
            f.write(fileContent)

        return {'FINISHED'}

def createBlenderMenu(self, context):
    self.layout.operator(Qt3DPathExporter.bl_idname, text="Qt3D Path Export(.json)")

# Register against Blender
def register():
    bpy.utils.register_class(Qt3DPathExporter)
    bpy.types.INFO_MT_file_export.append(createBlenderMenu)

def unregister():
    bpy.utils.unregister_class(Qt3DPathExporter)
    bpy.types.INFO_MT_file_export.remove(createBlenderMenu)

# Handle running the script from Blender's text editor.
if (__name__ == "__main__"):
   register();
   bpy.ops.export_scene.qt3d_path_exporter();
