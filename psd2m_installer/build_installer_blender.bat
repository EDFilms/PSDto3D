echo %date%  - %time%

call build_version.bat

@echo Creating output directories ...
mkdir ..\builds\installer\stage_full\blender\
mkdir ..\builds\installer\stage_full\blender\platforms
mkdir ..\builds\installer\stage_full\blender\imageformats
mkdir ..\builds\installer\stage_full\blender\icons\
mkdir ..\builds\installer\stage_full\docs\

@echo Updating file property resources ...
copy ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc.BACKUP
findreplace PLUGIN_DESC_TOKEN "PSDto3D Version %PLUGIN_VER_BASE%, Build%BUILD_VER%, Date %BUILD_DATE%" ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_YEAR_TOKEN %PLUGIN_YEAR% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\src\blender\__init__.py ..\builds\installer\stage_full\blender\__init__.py
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER_BASE% ..\psd2m_core\src\blender\blender_manifest.toml ..\builds\installer\stage_full\blender\blender_manifest.toml

@echo Build Blender plugin ...
SET _CL_=/DPSDTO3D_STANDALONE /DPSDTO3D_FULL_VERSION /DPSDTO3D_ATLAS_VERSION /O2 /Ob2 /Ot

devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Blender /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Blender /project psd2m_core

@echo Staging files, Blender version ...
copy ..\builds\plugin\RelWithDebInfo_Blender\PSDto3D_Standalone_dev.dll		 ..\builds\installer\stage_full\blender\PSDto3D_Standalone_%PLUGIN_VER%.dll
copy ..\builds\plugin\RelWithDebInfo_Blender\Qt?Core.dll			 ..\builds\installer\stage_full\blender\
copy ..\builds\plugin\RelWithDebInfo_Blender\Qt?Gui.dll				 ..\builds\installer\stage_full\blender\
copy ..\builds\plugin\RelWithDebInfo_Blender\Qt?Widgets.dll			 ..\builds\installer\stage_full\blender\
copy ..\builds\plugin\RelWithDebInfo_Blender\libGLESv2.dll			 ..\builds\installer\stage_full\blender\
copy ..\builds\plugin\RelWithDebInfo_Blender\platforms\qdirect2d.dll		 ..\builds\installer\stage_full\blender\platforms\
copy ..\builds\plugin\RelWithDebInfo_Blender\platforms\qminimal.dll		 ..\builds\installer\stage_full\blender\platforms\
copy ..\builds\plugin\RelWithDebInfo_Blender\platforms\qoffscreen.dll		 ..\builds\installer\stage_full\blender\platforms\
copy ..\builds\plugin\RelWithDebInfo_Blender\platforms\qwindows.dll		 ..\builds\installer\stage_full\blender\platforms\
copy ..\builds\plugin\RelWithDebInfo_Blender\imageformats\qgif.dll		 ..\builds\installer\stage_full\blender\imageformats\
copy ..\builds\plugin\RelWithDebInfo_Blender\imageformats\qico.dll		 ..\builds\installer\stage_full\blender\imageformats\
copy ..\builds\plugin\RelWithDebInfo_Blender\imageformats\qjpeg.dll		 ..\builds\installer\stage_full\blender\imageformats\
copy "..\psd2m_core\icons\PSD to 3D Icons.ico"					"..\builds\installer\stage_full\blender\icons\PSD to 3D Icons.ico"

@echo Deleting temp files ...
del ..\psd2m_core\src\psd2m_core.rc
move ..\psd2m_core\src\psd2m_core.rc.BACKUP ..\psd2m_core\src\psd2m_core.rc

"C:\Program Files\7-zip\7z.exe" a -r ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Blender_bin.zip ..\builds\installer\stage_full\blender\*

echo %date%  - %time%
