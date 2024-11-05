@rem ---------- ---------- ---------- ----------
@rem Run this to begin development within Unreal
@rem Creates an Unreal Editor project setup, and 
@rem Visual Studio project files.  Launch using:
@rem  x64 Native Tools Command Prompt for VS2022
@rem ---------- ---------- ---------- ----------

@rem make sure we execute in the same directory where this batch file lives
@pushd %~dp0

@rem Copying headers into project...
set PSDto3DHeaders=.\Plugins\Editor\PSDtoUnreal\Source\ThirdParty\PSDto3DLibrary\Public
@call :COPY_READ_ONLY ..\psd2m_core\src\  compLayers.h  %PSDto3DHeaders%\
@call :COPY_READ_ONLY ..\psd2m_core\src\  IPluginOutput.h  %PSDto3DHeaders%\
@call :COPY_READ_ONLY ..\psd2m_core\src\  outputs.h  %PSDto3DHeaders%\

@call :COPY_READ_ONLY ..\psd2m_mesh_generator\src\mesh_generator\  dataMesh.h  %PSDto3DHeaders%\mesh_generator\
@call :COPY_READ_ONLY ..\psd2m_mesh_generator\src\mesh_generator\linear_mesh\  bezierCurve.h  %PSDto3DHeaders%\linear_mesh\

@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  colorModeReader.h  %PSDto3DHeaders%\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  headerReader.h  %PSDto3DHeaders%\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  imageDataReader.h  %PSDto3DHeaders%\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  imageResourceReader.h  %PSDto3DHeaders%\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  layerAndMaskReader.h  %PSDto3DHeaders%\psd_reader\
@call :COPY_READ_ONLY ..\psd2m_psd_reader\src\psd_reader\  psdReader.h  %PSDto3DHeaders%\psd_reader\

@call :COPY_READ_ONLY ..\psd2m_util\src\util\  bounds_2D.h  %PSDto3DHeaders%\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  helpers.h  %PSDto3DHeaders%\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  math_2D.h  %PSDto3DHeaders%\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  progressJob.h  %PSDto3DHeaders%\util\
@call :COPY_READ_ONLY ..\psd2m_util\src\util\  vectorialPath.h  %PSDto3DHeaders%\util\

@popd
@goto :eof

@rem Subroutines

:COPY_READ_ONLY
@rem Param1: source dir, Param2: source filename, Param3: dest dir
@IF EXIST %3\%2 attrib -r %3\%2
xcopy /Y %1\%2 %3
@attrib +r %3\%2
@EXIT /B
