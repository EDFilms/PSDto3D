@rem ---------- ---------- ---------- ----------
@rem Run this to begin development within Unreal
@rem Creates an Unreal Editor project setup, and 
@rem Visual Studio project files.  Launch using:
@rem  x64 Native Tools Command Prompt for VS2022
@rem ---------- ---------- ---------- ----------

@echo Building PSDto3D binaries ...
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Unreal /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Unreal /project psd2m_core

@rem Files copied into place via project build actions
