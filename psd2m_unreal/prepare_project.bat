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

@rem Copying headers into project...
@call :COPY_READ_ONLY ..\psd2m_core\src\  compLayers.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\
@call :COPY_READ_ONLY ..\psd2m_core\src\  IPluginOutput.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\
@call :COPY_READ_ONLY ..\psd2m_core\src\  outputs.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\

@call :COPY_READ_ONLY ..\psd2m_mesh_generator\src\mesh_generator\  dataMesh.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\mesh_generator\
@call :COPY_READ_ONLY ..\psd2m_mesh_generator\src\mesh_generator\linear_mesh\  bezierCurve.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\linear_mesh\

@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  colorModeReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  headerReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  imageDataReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  imageResourceReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  layerAndMaskReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  psdReader.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\psd_reader\

@call :COPY_READ_ONLY ..\psd2m_util\src\util\  bounds_2D.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  math_2D.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  progressJob.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  vectorialPath.h  .\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public\PSDto3DLibrary\util\

@popd
@goto :eof

@rem Subroutines

:COPY_READ_ONLY
@rem Param1: source dir, Param2: source filename, Param3: dest dir
@IF EXIST %3\%2 attrib -r %3\%2
xcopy /Y %1\%2 %3
@attrib +r %3\%2
@EXIT /B
