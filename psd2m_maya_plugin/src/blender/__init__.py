
bl_info = {
    "name": "PSDto3D",
    "author": "EDFilms",
    "version": (0, 1, 0),
    "blender":  (2, 80, 0),
    "location": "View3D > Add > Mesh",
    "description": "PSDto3D",
    "wiki_url": "http://edfilms.net/",
    "category": "Object",
}

import bpy
from bpy.types import Operator
from bpy_extras.object_utils import AddObjectHelper
#from bpy.props import (
#        BoolProperty,
#        EnumProperty,
#        FloatProperty,
#        IntProperty,
#        FloatVectorProperty,
#        StringProperty,
#        )
from bpy_extras import object_utils


#class PSDto3D(bpy.types.Operator):
class PSDto3D(Operator, AddObjectHelper):
    """PSDto3D Editor Window"""      # blender will use this as a tooltip for menu items and buttons.
    bl_idname = "object.psd_to_3d"        # unique identifier for buttons and menu items to reference.
    bl_label = "PSDto3D"         # display name in the interface.
    bl_options = {'REGISTER', 'UNDO'}  # enable undo for the operator.
    bl_description = "PSDto3D"    

    #### change properties
#    name : StringProperty(name = "Name",
#                    description = "Name")

    def draw(self, context):
        return {'FINISHED'}            # this lets blender know the operator finished successfully.

#    @classmethod
#    def poll(cls, context):
#        return context.scene is not None
               
    def execute(self, context):        # execute() is called by blender when running the operator.
        import os
        import ctypes
        os.environ["QT_PLUGIN_PATH"] = r"C:\WORKSPACE\DEPOT\Builds\plugin\RelWithDebInfo_MayaStub"+'\\'
        os.environ["PATH"] = os.environ["PATH"] + r";C:\WORKSPACE\DEPOT\Builds\plugin\RelWithDebInfo_MayaStub"+'\\'
        testDll = ctypes.WinDLL (r"C:\WORKSPACE\DEPOT\Builds\plugin\RelWithDebInfo_MayaStub\PSDto3D_Maya2020_dev.mll")
        testApiProto = ctypes.WINFUNCTYPE ( ctypes.c_int, ctypes.c_int )
        testApiParams = None
        openPluginFn = testApiProto (("openPlugin", testDll), testApiParams)
        openPluginFn (0)

        # The original script
        scene = context.scene
        for obj in scene.objects:
            obj.location.x += 1.0

        return {'FINISHED'}            # this lets blender know the operator finished successfully.
        
    def invoke(self, context, event):
        self.execute(context)

        return {'FINISHED'}

def register():
    bpy.utils.register_class(PSDto3D)


def unregister():
    bpy.utils.unregister_class(PSDto3D)


# This allows you to run the script directly from blenders text editor
# to test the addon without having to install it.
if __name__ == "__main__":
    register()