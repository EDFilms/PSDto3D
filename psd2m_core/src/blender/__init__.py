import bpy
import bmesh
import os
import ctypes
import copy
import time
import threading
import pdb # for debugging

from bpy_extras import object_utils
from bpy_extras.object_utils import AddObjectHelper
from bpy.props import FloatProperty


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
        mat.node_tree.links.new(texImage_node.outputs[0], principled_bsdf.inputs[0])
        # conenct texture alpha to material alpha
        mat.node_tree.links.new(texImage_node.outputs[1], principled_bsdf.inputs[4])
        mat.blend_method = 'CLIP'
        bpy.psdToBlender_materials[texFilepath] = mat
    return mat


def psdToBlender_applyMaterial( obj, mat ):
    #print( f'applyMaterial() entry, obj:{obj} mat:{mat}' )
    # Assign it to object
    if obj.data.materials:
        # assign to 1st material slot`
        obj.data.materials[0] = mat
    else:
        # no slots
        obj.data.materials.append(mat)
        
    
def psdToBlender_callbackTask():
    markTime = time.perf_counter()
    dataMeshPtr = bpy.psdToBlender_taskParam
    dataMesh = dataMeshPtr[0]
    print( '----- PSD to 3D export -----' )

    print( f'Creating mesh: {dataMesh.meshName}' )
    print( f'Creating material: {dataMesh.materialName}' )
    #print( f'dataMesh.exportIndex(): {dataMesh.exportIndex}' )
    #print( f'dataMesh.layerIndex(): {dataMesh.layerIndex}' )
    #print( f'dataMesh.sceneScale(): {dataMesh.sceneScale}' )
    #print( f'dataMesh.sceneAspect(): {dataMesh.sceneAspect}' )
    #print( f'dataMesh.MeshName(): {dataMesh.meshName}' )
    #print( f'dataMesh.MeshVertCount(): {dataMesh.MeshVertCount(dataMesh.self)}' )
    #print( f'dataMesh.MeshFaceCount(): {dataMesh.MeshFaceCount(dataMesh.self)}' )
    
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
    #print( f'faces: {faces}' )
    #print( f'verts: {verts}' )
    #print( f'texcoords: {texcoords}' )

    mesh = bpy.data.meshes.new( dataMesh.meshName )
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

    # add the mesh as an object into the scene with this utility module
    obj = object_utils.object_data_add( bpy.context, mesh )
    mpos = dataMesh.MeshPosition(dataMesh.self)
    obj.location = (mpos.x, mpos.y, mpos.z)

    mat = psdToBlender_getMaterial(  dataMesh.materialName, dataMesh.textureFilepath )
    psdToBlender_applyMaterial( obj, mat )

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


class psdToBlender_tool( bpy.types.Operator ):
    '''PSD to Blender'''
    bl_idname = 'object.psd_to_blender'
    bl_label = 'PSD to Blender'
    def execute(self, context):
        #printObject( context )
        
        # load the DLL and install our callback, which also launches the UI
        # from example at https://gist.github.com/Nican/5198719
        script_file = os.path.realpath(__file__)
        pluginPath = os.path.dirname(script_file) + '\\'
        pluginFilename = 'PSDto3D_Standalone_PLUGIN_VER_TOKEN.dll'
        pluginFilepath = pluginPath + pluginFilename

        #debugging only
        if( pluginFilename.endswith('VER_TOKEN.dll') ): # if this isn't the installed version, use the test location
            pluginFilepath = pluginPath + 'PSDto3D_Standalone_dev.dll'

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


def psdToBlender_func( self, context ):
    self.layout.operator(psdToBlender_tool.bl_idname, text=psdToBlender_tool.bl_label)


# Register and add to the "Object" menu
def register():
    bpy.utils.register_class(psdToBlender_tool)
    bpy.types.VIEW3D_MT_object.append(psdToBlender_func)


def unregister():
    bpy.utils.unregister_class(psdToBlender_tool)
    bpy.types.VIEW3D_MT_object.remove(psdToBlender_func)


if __name__ == '__main__':
    register()


# end of file
