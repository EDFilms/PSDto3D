@rem ---------- ---------- ---------- ----------
@rem Run this to begin development within Unreal
@rem Creates an Unreal Editor project setup, and 
@rem Visual Studio project files.  Launch using:
@rem  x64 Native Tools Command Prompt for VS2022
@rem ---------- ---------- ---------- ----------

@rem make sure we execute in the same directory where this batch file lives
@pushd %~dp0

set QtDir=C:\build\qt5
set ProjectBinDir=.\Plugins\Editor\PSDtoUnreal\Binaries\ThirdParty\PSDto3DLibrary\Win64
@echo Building PSDto3D binaries ...
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Unreal /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Unreal /project psd2m_core

@echo Copying PSDto3D binaries into project ...
xcopy ..\builds\plugin\RelWithDebInfo_Unreal\PSDto3D_Standalone_dev.* %ProjectBinDir%\ /E /Y
rename %ProjectBinDir%\PSDto3D_Standalone_dev.dll PSDto3D_Standalone.dll
@rename %ProjectBinDir%\PSDto3D_Standalone_dev.pdb PSDto3D_Standalone.pdb

@echo Copying Qt binaries into project ...
xcopy /E /Y %QtDir%\bin\libGLESv?.dll                 %ProjectBinDir%\
xcopy /E /Y %QtDir%\bin\Qt?Core.dll                   %ProjectBinDir%\
xcopy /E /Y %QtDir%\bin\Qt?Gui.dll                    %ProjectBinDir%\
xcopy /E /Y %QtDir%\bin\Qt?Widgets.dll                %ProjectBinDir%\
xcopy /E /Y %QtDir%\plugins\imageformats\qgif.dll     %ProjectBinDir%\imageformats\
xcopy /E /Y %QtDir%\plugins\imageformats\qico.dll     %ProjectBinDir%\imageformats\
xcopy /E /Y %QtDir%\plugins\imageformats\qjpeg.dll    %ProjectBinDir%\imageformats\
xcopy /E /Y %QtDir%\plugins\platforms\qdirect2d.dll   %ProjectBinDir%\platforms\
xcopy /E /Y %QtDir%\plugins\platforms\qminimal.dll    %ProjectBinDir%\platforms\
xcopy /E /Y %QtDir%\plugins\platforms\qoffscreen.dll  %ProjectBinDir%\platforms\
xcopy /E /Y %QtDir%\plugins\platforms\qwindows.dll    %ProjectBinDir%\platforms\

@popd
