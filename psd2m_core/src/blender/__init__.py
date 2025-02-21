import bpy
import bmesh
import os
import ctypes
import copy
import time
import threading
import pdb # for debugging

from bpy.types import Operator
from bpy.types import Panel
from bpy_extras import object_utils
from bpy_extras.object_utils import AddObjectHelper
from bpy.props import FloatProperty
from ctypes.util import find_library


def printObject(obj):
    print( type(obj) )
    print( dir(obj) )
    

# This function can safely be called in another thread
# Blocks until the function is finished executing in the main thread
def psdToBlender_runTask(function):
    markTime = time.perf_counter()
    bpy.psdToBlender_lock.acquire() # task is a singleton;  prevent more than one from being queued
    bpy.psdToBlender_task = function
    while( bpy.psdToBlender_lock.locked() ):
        time.sleep( 0.01 ) # wait for the task to complete
    elapsed = time.perf_counter() - markTime
    print(f'  total time ... {elapsed:.6f} seconds')


# Runs the available tasks, executed on each main thread timer tick
def psdToBlender_runTasks():
    if( bpy.psdToBlender_task != None ):
        function = bpy.psdToBlender_task
        function()
        bpy.psdToBlender_task = None # end the task
        bpy.psdToBlender_lock.release() # release lock so calling thread can continue
    return 0.05 # next delay 5ms; trigger up to 20 times per second


def psdToBlender_getMaterial( matName, texFilepath ):
    mat = None
    # if the name-to-material lookup dictionary needs to be created
    if( not hasattr( bpy, 'psdToBlender_materials' ) ):
        bpy.psdToBlender_materials = {}
    # fetch material from dictionary or create a new one
    if( texFilepath in bpy.psdToBlender_materials ):
        mat = bpy.psdToBlender_materials[texFilepath]
    else:
        # create material
        mat = bpy.data.materials.new(name= matName)
        mat.use_nodes = True

        #create a reference to the material output
        principled_bsdf = mat.node_tree.nodes.get('Principled BSDF')

        texImage_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
        texImage_node.image = bpy.data.images.load( texFilepath )

        # connect texture color to material base color
        mat.node_tree.links.new( texImage_node.outputs[0], principled_bsdf.inputs[0] )
        # conenct texture alpha to material alpha
        mat.node_tree.links.new( texImage_node.outputs[1], principled_bsdf.inputs[4] )
        mat.blend_method = 'CLIP'
        bpy.psdToBlender_materials[texFilepath] = mat
    return mat


def psdToBlender_applyMaterial( obj, mat ):
    # Assign it to object
    if obj.data.materials:
        # assign to 1st material slot`
        obj.data.materials[0] = mat
    else:
        # no slots
        obj.data.materials.append(mat)
        
# replace mesh in scene
def psdToBlender_replaceMesh( mesh, meshName ):
    print(f'replacing existing mesh ...')
    obj = bpy.data.objects[ meshName ]
    oldMesh = obj.data
    obj.data = mesh
    bpy.data.meshes.remove(oldMesh)
    obj.data.name = meshName
    return obj
    
def psdToBlender_callbackTask():
    markTime = time.perf_counter()
    dataMeshPtr = bpy.psdToBlender_taskParam
    dataMesh = dataMeshPtr[0]
    print( '----- PSD to 3D export -----' )

    print( f'Creating mesh: {dataMesh.meshName}' )
    print( f'Creating material: {dataMesh.materialName}' )
    
    # for each face, create a tuple listing the vert indices for the face
    faces = []
    for fi in range( 0,dataMesh.MeshFaceCount(dataMesh.self) ):
        fv = ()
        for fvi in range( 0,dataMesh.MeshFaceVertCount(dataMesh.self,fi) ):
            fv += ( dataMesh.MeshFaceVert(dataMesh.self,fi,fvi), ) # convert number to tuple and append
        faces += (fv,) # append tuple to list of faces

    # for each vert, create a tuple for the (x,y,z) position and a tuple for the (u,v) texture coords
    verts = []
    texcoords = []
    for vi in range( 0,dataMesh.MeshVertCount(dataMesh.self) ):
        vpos = dataMesh.MeshVertPos(dataMesh.self,vi)
        vtex = dataMesh.MeshVertUV(dataMesh.self,vi)
        verts += ((vpos.x, vpos.y, vpos.z), ) # wrap the tuple in a tuple, otherwise this is three appends
        texcoords  += ((vtex.x, vtex.y), ) # wrap the tuple in a tuple, otherwise this is three appends

    meshExisted = False
    meshName = (dataMesh.psdName + '.' + dataMesh.meshName)
    if( meshName in bpy.data.objects ):
        meshExisted = True
        
    mesh = bpy.data.meshes.new( meshName )        
    bm = bmesh.new()

    for v_co in verts:
        bm.verts.new(v_co)
    bm.verts.ensure_lookup_table()
    uv_layer = bm.loops.layers.uv.verify()
    
    #pdb.set_trace() # begin debugging
    # create each face based on vert indices and texture coord values
    for fi in range(0,len(faces)):
        fvs = faces[fi]
        # set face vert indices
        face = bm.faces.new([bm.verts[i] for i in fvs])
        # set face uv values
        for fvi in range(0,len(fvs)):
            loop = face.loops[fvi]
            loop_uv = loop[uv_layer]
            loop_uv.uv = texcoords[ fvs[fvi] ]
    #bmesh.update_edit_mesh(me) # if editing a previously created mesh

    elapsed = time.perf_counter() - markTime
    print(f'creating mesh ... {elapsed:.6f} seconds')

    bm.to_mesh(mesh)
    mesh.update()

    elapsed = time.perf_counter() - markTime
    print(f'creating material ... {elapsed:.6f} seconds')

    obj = None
    if( meshExisted ): # mesh already exists, update it ...
        obj = psdToBlender_replaceMesh( mesh, meshName ) # fetch and replace mesh in scene
    if( not meshExisted ): # ... else, add the mesh as a new object into the scene
        obj = object_utils.object_data_add( bpy.context, mesh ) # use utility module to create mesh
        # only set object position if it's a new object, user may have positioned it otherwise
        mpos = dataMesh.MeshPosition(dataMesh.self)
        obj.location = (mpos.x, mpos.y, mpos.z)
    # if user deleted original mesh, meshExisted will be false,
    # but the object still receives an incremented name, like myObject.002,
    # so re-applying the name change here works around that    
    obj.name = meshName

    elapsed = time.perf_counter() - markTime
    print(f'creating object ... {elapsed:.6f} seconds')
    
    mat = psdToBlender_getMaterial( dataMesh.materialName, dataMesh.textureFilepath )
    psdToBlender_applyMaterial( obj, mat )
    
    # if refreshing an object, texture may be an atlas that has changed, so reload it from disk
    print(f'textureFilepath ... {dataMesh.textureFilepath}')
    textureFilename = dataMesh.textureFilepath.split('\\')[-1]
    if( meshExisted and (textureFilename in bpy.data.images) ):
        print(f'reloading existing texture ... {textureFilename}')
        bpy.data.images[textureFilename].reload()

    elapsed = time.perf_counter() - markTime
    return 0


def psdToBlender_callback( dataMeshPtr ):
    '''
    Called once per mesh, after export operation is complete in PSDtoBlender.
    Actual mesh data creation within Blender is done here.
    Input parameter is mesh data object.
    '''
    bpy.psdToBlender_taskParam = dataMeshPtr
    psdToBlender_runTask( psdToBlender_callbackTask )
    return 0


class psdToBlender_OT( bpy.types.Operator ):
    '''PSD to 3D'''
    bl_idname = 'object.psd_to_blender'
    bl_label = 'Open PSD to 3D Window'
    def execute(self, context):
        #printObject( context )
        
        # load the DLL and install our callback, which also launches the UI
        # from example at https://gist.github.com/Nican/5198719
        script_file = os.path.realpath(__file__)
        pluginPath = os.path.dirname(script_file) + '\\'
        pluginFilename = 'PSDto3D_Standalone_PLUGIN_VER_TOKEN.dll'
        pluginFilepath = ctypes.util.find_library( pluginPath + pluginFilename )

        #debugging only
        if( pluginFilepath == None ): # if this isn't the installed version, use the dev version
            pluginFilepath = ctypes.util.find_library( pluginPath + 'PSDto3D_Standalone_dev.dll' )

        # describe the Vector3F struct as defined in the plugin DLL
        class Vector3F( ctypes.Structure ):
            _fields_ = [
                ( 'x', ctypes.c_float ),
                ( 'y', ctypes.c_float ),
                ( 'z', ctypes.c_float )
            ]
        ctype_pluginIntVoidFn = ctypes.CFUNCTYPE( ctypes.c_int, ctypes.c_void_p ) # returns an int, takes (void)
        ctype_pluginIntIntFn = ctypes.CFUNCTYPE( ctypes.c_int, ctypes.c_void_p, ctypes.c_int ) # returns an int, takes (int)
        ctype_pluginIntIntIntFn = ctypes.CFUNCTYPE( ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_int ) # returns an int, takes (int,int)
        ctype_pluginVector3IntFn = ctypes.CFUNCTYPE( Vector3F, ctypes.c_void_p, ctypes.c_int ) # returns an Vector3F, takes (int)
        ctype_pluginVector3VoidFn = ctypes.CFUNCTYPE( Vector3F, ctypes.c_void_p ) # returns an Vector3F, takes (int)
        # describe the DataMesh class as defined in the plugin DLL
        class PluginDataMesh( ctypes.Structure ):
            _fields_ = [
                ( 'self', ctypes.c_void_p ), # pointer to self
                ( 'paramData', ctypes.c_void_p ), # IPluginOutputParameters
                ( 'psdData', ctypes.c_void_p ), # PsdData
                ( 'layerData', ctypes.c_void_p ), # GraphLayer
                
                ( 'psdName', ctypes.c_wchar_p ),
                ( 'exportIndex', ctypes.c_int ),
                ( 'layerIndex', ctypes.c_int ),
                ( 'sceneScale', ctypes.c_float ),
                ( 'sceneDepth', ctypes.c_float ),
                ( 'sceneAspect', ctypes.c_float ), # Aspect ratio of the PSD file
                ( 'meshName', ctypes.c_wchar_p ),
                ( 'materialName', ctypes.c_wchar_p ),
                ( 'textureFilepath', ctypes.c_wchar_p ),
                
                ( 'MeshPosition', ctype_pluginVector3VoidFn ),
                ( 'MeshVertCount', ctype_pluginIntVoidFn ),
                ( 'MeshFaceCount', ctype_pluginIntVoidFn ),
                ( 'MeshFaceVertCount', ctype_pluginIntIntFn ),
                ( 'MeshFaceVert', ctype_pluginIntIntIntFn ),
                ( 'MeshVertPos', ctype_pluginVector3IntFn ),
                ( 'MeshVertUV', ctype_pluginVector3IntFn )
            ]

        # launch the task processing queue
        # performs scene operations in the main thread
        if( not hasattr( bpy, 'psdToBlender_tasks' ) ):
           bpy.psdToBlender_lock = threading.Lock()
           bpy.psdToBlender_task = None
           bpy.app.timers.register( psdToBlender_runTasks )

        # load the DLL
        dll = ctypes.CDLL( pluginFilepath )
        # define the type of the callback function
        # use CFUNCTYPE for __cdecl and WINFUNCTYPE for __stdcall in plugin linker settings
        # describe the addListener fucntion and its callback param
        # DEBUGGING: Use Blender Window menu->Toggle System Console to see output
        ctype_setCallback = ctypes.CFUNCTYPE( ctypes.c_int, ctypes.POINTER(PluginDataMesh) )
        dll.pythonSetCallback.argtypes = [ ctype_setCallback ]
        dll.pythonSetCallback.restype = ctypes.c_void_p
        # launch dialog by calling pythonSetCallback() function in the DLL
        bpy.psdToBlender_callback = ctype_setCallback( psdToBlender_callback ) # store callback instance in global scope

        dll.pythonSetCallback( bpy.psdToBlender_callback )
            
        return {'FINISHED'}



class psdToBlender_PT(bpy.types.Panel):
    bl_label = "PSD to 3D"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "PSD to 3D"

    def draw(self, context):
        self.layout.operator( psdToBlender_OT.bl_idname )


def register():
    bpy.utils.register_class(psdToBlender_OT)
    bpy.utils.register_class(psdToBlender_PT)


def unregister():
    bpy.utils.unregister_class(psdToBlender_OT)
    bpy.utils.unregister_class(psdToBlender_PT)



if __name__ == '__main__':
    register()


# end of file
