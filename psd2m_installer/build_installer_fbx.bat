echo %date%  - %time%

set BUILD_VER=075
set BUILD_DATE=%date%
set PLUGIN_VER_SHORT=165
set PLUGIN_VER_BASE=1.6.5
set PLUGIN_VER=%PLUGIN_VER_BASE%.%BUILD_VER%
set PLUGIN_YEAR=2023

@echo Updating file property resources ...
copy ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc.BACKUP
findreplace PLUGIN_DESC_TOKEN "PSDto3D Version %PLUGIN_VER_BASE%, Build%BUILD_VER%, Date %BUILD_DATE%" ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc
findreplace PLUGIN_YEAR_TOKEN %PLUGIN_YEAR% ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc

@echo Building Photoshop CC extension ...
pushd ..\psd2m_extension\PsdExporterCC\
call build_extension_cc.bat
popd

@echo Building Photoshop CS6 extension ...
pushd ..\psd2m_extension\PsdExporterCS6\
call build_extension_cs6.bat
popd


@echo Build Maya plugin, Pro version ...
SET _CL_=/DPSDTO3D_STANDALONE /DPSDTO3D_FULL_VERSION /DPSDTO3D_ATLAS_VERSION /O2 /Ob2 /Ot

@rem mkdir ..\builds\plugin\RelWithDebInfo_Standalone
mkdir ..\builds\installer\stage_full\fbx\
mkdir ..\builds\installer\stage_full\fbx\platforms
mkdir ..\builds\installer\stage_full\fbx\imageformats
mkdir ..\builds\installer\stage_full\fbx\icons\
mkdir ..\builds\installer\stage_full\docs\

devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_FBX /project psd2m_maya_plugin
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_FBX /project psd2m_fbx
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_FBX /project psd2m_maya_plugin
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_FBX /project psd2m_fbx
copy ..\builds\plugin\RelWithDebInfo_Standalone\PSDto3D_FBX_dev.exe ..\builds\installer\stage_full\fbx\PSDtoFBX_edfilms_%PLUGIN_VER%.exe
copy ..\builds\plugin\RelWithDebInfo_Standalone\PSDto3D_Standalone_dev.dll ..\builds\installer\stage_full\fbx\PSDto3D_Standalone_%PLUGIN_VER%.dll

@echo Staging files, FBX version ...
copy ..\builds\plugin\RelWithDebInfo_Standalone\Qt5Core.dll ..\builds\installer\stage_full\fbx\Qt5Core.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\Qt5Gui.dll ..\builds\installer\stage_full\fbx\Qt5Gui.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\Qt5Widgets.dll ..\builds\installer\stage_full\fbx\Qt5Widgets.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\libGLESv2.dll ..\builds\installer\stage_full\fbx\libGLESv2.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\platforms\qdirect2d.dll ..\builds\installer\stage_full\fbx\platforms\qdirect2d.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\platforms\qminimal.dll ..\builds\installer\stage_full\fbx\platforms\qminimal.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\platforms\qoffscreen.dll ..\builds\installer\stage_full\fbx\platforms\qoffscreen.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\platforms\qwindows.dll ..\builds\installer\stage_full\fbx\platforms\qwindows.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\imageformats\qgif.dll ..\builds\installer\stage_full\fbx\imageformats\qgif.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\imageformats\qico.dll ..\builds\installer\stage_full\fbx\imageformats\qico.dll
copy ..\builds\plugin\RelWithDebInfo_Standalone\imageformats\qjpeg.dll ..\builds\installer\stage_full\fbx\imageformats\qjpeg.dll
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" "..\builds\installer\stage_full\fbx\icons\PSD to 3D Icons.ico"
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" ".\TEMP_PSDto3D_INSTALLER.ico"
copy "..\psd2m_maya_plugin\docs\Online Documentation.url" "..\builds\installer\stage_full\docs\Online Documentation.url"

mkdir ..\builds\installer\stage_full\fbx\conf\english
mkdir ..\builds\installer\stage_full\fbx\conf\french
copy ..\psd2m_maya_plugin\conf\language_english.ini ..\builds\installer\stage_full\fbx\conf\english\language.ini
copy ..\psd2m_maya_plugin\conf\language_french.ini ..\builds\installer\stage_full\fbx\conf\french\language.ini

xcopy ..\builds\photoshop ..\Builds\installer\stage_full\photoshop\ /E /Y
move ..\builds\photoshop ..\Builds\installer\stage_full\photoshop\ /E /Y

mkdir ..\builds\installer\output
mkdir ..\builds\installer\installers

@echo Building installer, FBX version
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ./psd2fbx_full.iss ./TEMP_psd2fbx_full_%PLUGIN_VER%.iss
.\InnoSetup602\compil32.exe /cc ./TEMP_psd2fbx_full_%PLUGIN_VER%.iss
move .\Output\setup.exe ..\builds\installer\installers\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_FBX_setup.exe

@echo Deleting temp files ...
del ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc
move ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc.BACKUP ..\psd2m_maya_plugin\src\psd2m_maya_plugin.rc
del ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_FBX_bin_password_edfilms.7z
"C:\Program Files\7-zip\7z.exe" a -pedfilms -r ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_FBX_bin_password_edfilms.7z ..\builds\installer\stage* ..\builds\installer\installer*

echo %date%  - %time%
