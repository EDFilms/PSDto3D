echo %date%  - %time%

set BUILD_VER=077
set BUILD_DATE=%date%
set PLUGIN_VER_SHORT=165
set PLUGIN_VER_BASE=1.6.5
set PLUGIN_VER=%PLUGIN_VER_BASE%.%BUILD_VER%
set PLUGIN_YEAR=2024
set UNREAL_DIR=C:\BUILD\Unreal\

@rem Check for Unreal and Visual Studio
@IF NOT EXIST %UNREAL_DIR%\Engine\Binaries\Win64\UnrealEditor.exe (
    echo Cannot find Unreal binaries.  Edit this script and update UNREAL_DIR
    @goto :eof
)
@IF "%VisualStudioVersion%"=="" (
    echo Cannot find Visual Studio.  Launch this script from the x64 Native Tools Command Prompt for VS2022
    @goto :eof
)
@IF NOT "%VisualStudioVersion%"=="17.0" (
    echo Warning: Wrong version of Visual Studio detected.  Launch this script from the x64 Native Tools Command Prompt for VS2022
)

@echo Updating file property resources ...
copy ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc.BACKUP
findreplace PLUGIN_DESC_TOKEN "PSDto3D Version %PLUGIN_VER_BASE%, Build%BUILD_VER%, Date %BUILD_DATE%" ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_YEAR_TOKEN %PLUGIN_YEAR% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc

@echo Building Unreal plugin for packaging ...
@rem This also builds using the following:
@rem   devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Unreal /project psd2m_core
call ..\psd2m_unreal\prepare_project.bat

@echo Packaging plugin ...
mkdir ..\Builds\installer\stage_full\unreal
call %UNREAL_DIR%\Engine\Build\BatchFiles\RunUAT.bat BuildPlugin -Plugin="%~dp0\..\psd2m_unreal\Plugins\Editor\PSDtoUnreal\PSDtoUnreal.uplugin" -Package="%~dp0\..\Builds\installer\stage_full\unreal" -Rocket

@echo Deleting temp files ...
del ..\psd2m_core\src\psd2m_core.rc
move ..\psd2m_core\src\psd2m_core.rc.BACKUP ..\psd2m_core\src\psd2m_core.rc
del ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Unreal_bin_password_edfilms.7z
"C:\Program Files\7-zip\7z.exe" a -pedfilms -r ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Unreal_bin_password_edfilms.7z ..\builds\installer\stage* ..\builds\installer\installer*

echo %date%  - %time%