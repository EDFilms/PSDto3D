echo %date%  - %time%

set BUILD_VER=075
set BUILD_DATE=%date%
set PLUGIN_VER_SHORT=165
set PLUGIN_VER_BASE=1.6.5
set PLUGIN_VER=%PLUGIN_VER_BASE%.%BUILD_VER%
set PLUGIN_YEAR=2023
set UNREAL_BUILD=23058290

mkdir ..\builds\installer\stage_full\unreal\
mkdir ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\platforms
mkdir ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\imageformats
xcopy ..\psd2m_maya_plugin\src\unreal\deploy ..\builds\installer\stage_full\unreal\ /E /Y


@echo Updating file property resources ...
findreplace UNREAL_BUILD_TOKEN %UNREAL_BUILD% ..\psd2m_maya_plugin\src\unreal\deploy\PSDtoUnreal\Binaries\Win64\UnrealEditor.modules ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\UnrealEditor.modules

@echo Build Unreal plugin ...
SET _CL_=/DPSDTO3D_STANDALONE /DPSDTO3D_FULL_VERSION /DPSDTO3D_ATLAS_VERSION /O2 /Ob2 /Ot

SET BuildDir=%UnrealDir%\Engine\Plugins\Editor\PSDtoUnreal\Binaries\Win64

devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Unreal /project psd2m_maya_plugin
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Unreal /project psd2m_unreal_plugin
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Unreal /project psd2m_maya_plugin
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Unreal /project psd2m_unreal_plugin
copy %BuildDir%\PSDto3D_Unreal_dev.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\
copy %BuildDir%\PSDto3D_Standalone_dev.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\

@echo Staging files, Unreal version ...
copy %BuildDir%\Qt5Core.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\Qt5Core.dll
copy %BuildDir%\Qt5Gui.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\Qt5Gui.dll
copy %BuildDir%\Qt5Widgets.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\Qt5Widgets.dll
copy %BuildDir%\libGLESv2.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\libGLESv2.dll
copy %BuildDir%\platforms\qdirect2d.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\platforms\qdirect2d.dll
copy %BuildDir%\platforms\qminimal.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\platforms\qminimal.dll
copy %BuildDir%\platforms\qoffscreen.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\platforms\qoffscreen.dll
copy %BuildDir%\platforms\qwindows.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\platforms\qwindows.dll
copy %BuildDir%\imageformats\qgif.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\imageformats\qgif.dll
copy %BuildDir%\imageformats\qico.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\imageformats\qico.dll
copy %BuildDir%\imageformats\qjpeg.dll ..\builds\installer\stage_full\unreal\PSDtoUnreal\Binaries\Win64\imageformats\qjpeg.dll
copy "..\psd2m_maya_plugin\docs\Online Documentation.url" "..\builds\installer\stage_full\unreal\PSDtoUnreal\Online Documentation.url"
