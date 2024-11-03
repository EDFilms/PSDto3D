@rem ---------- ---------- ---------- ----------
@rem Run this to begin development within Unreal
@rem Creates Visual Studio project files
@rem ---------- ---------- ---------- ----------


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

@echo Generating Visual Studio project files ...
call %UNREAL_DIR%\Engine\Build\BatchFiles\GetDotnetPath.bat
dotnet "%UNREAL_DIR%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" -projectfiles -project="%~dp0\HostProject.uproject" -game -rocket -progress

@echo %UNREAL_DIR%\Engine\Binaries\Win64\UnrealEditor.exe >.\LaunchUnrealEditor.bat

