import os
import ctypes
baseDir = r"C:\WORKSPACE\DEPOT\UNREAL\Engine\Plugins\Editor\PSDtoUnreal\Binaries\Win64"
os.environ["QT_PLUGIN_PATH"] = baseDir+'\\'
os.environ["PATH"] = baseDir+'\\'
testDll = ctypes.WinDLL( baseDir + r"\PSDto3D_Standalone_dev.dll" )
testApiProto = ctypes.WINFUNCTYPE ( ctypes.c_int, ctypes.c_int )
testApiParams = None
openPluginFn = testApiProto (("openPlugin", testDll), testApiParams)
openPluginFn (0)
