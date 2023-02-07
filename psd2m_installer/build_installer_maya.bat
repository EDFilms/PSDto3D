echo %date%  - %time%

set PLUGIN_VER=1.6.3
set PLUGIN_VER_SHORT=163
set BUILD_VER=069

@echo Building Photoshop CC extension ...
pushd ..\psd2m_extension\PsdExporterCC\
call build_extension_cc.bat
popd

@echo Building Photoshop CS6 extension ...
pushd ..\psd2m_extension\PsdExporterCS6\
call build_extension_cs6.bat
popd


@echo Build Maya plugin, Pro version ...
SET _CL_=/DPSDTO3D_FULL_VERSION /O2 /Ob2 /Ot

@rem mkdir ..\builds\plugin\RelWithDebInfo_2023_Full
mkdir ..\builds\installer\stage_full\maya\2023\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2023 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2023 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2023\PSDto3D_Maya2023_dev.mll ..\builds\installer\stage_full\maya\2023\PSDto3D_Maya2023_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2022_Full
mkdir ..\builds\installer\stage_full\maya\2022\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2022 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2022 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2022\PSDto3D_Maya2022_dev.mll ..\builds\installer\stage_full\maya\2022\PSDto3D_Maya2022_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2020_Full
mkdir ..\builds\installer\stage_full\maya\2020\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2020 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2020 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2020\PSDto3D_Maya2020_dev.mll ..\builds\installer\stage_full\maya\2020\PSDto3D_Maya2020_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2019_Full
mkdir ..\builds\installer\stage_full\maya\2019\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2019 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2019 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2019\PSDto3D_Maya2019_dev.mll ..\builds\installer\stage_full\maya\2019\PSDto3D_Maya2019_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2018_Full
mkdir ..\builds\installer\stage_full\maya\2018\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2018 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2018 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2018\PSDto3D_Maya2018_dev.mll ..\builds\installer\stage_full\maya\2018\PSDto3D_Maya2018_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2017_Full
@rem mkdir ..\builds\installer\stage_full\maya\2017\
@rem devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2017 /project psd2m_maya_plugin
@rem devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2017 /project psd2m_maya_plugin
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2017\PSDto3D_Maya2017_dev.mll ..\builds\installer\stage_full\maya\2017\PSDto3D_Maya2017_%PLUGIN_VER%.mll

@echo Staging files, Pro version ...
xcopy ..\builds\photoshop ..\Builds\installer\stage_full\photoshop\ /E /Y
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2023\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2023\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2022\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2022\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2020\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2020\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2019\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2019\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2018\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2018\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2017\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2017\shelf_PSDto3D.mel

mkdir ..\builds\installer\stage_full\maya\icons
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" "..\builds\installer\stage_full\maya\icons\PSD to 3D Icons.ico"
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" ".\TEMP_PSDto3D_INSTALLER.ico"

mkdir ..\builds\installer\stage_full\docs
copy "..\psd2m_maya_plugin\docs\Online Documentation.url" "..\builds\installer\stage_full\docs\Online Documentation.url"

mkdir ..\builds\installer\stage_full\maya\conf\english
mkdir ..\builds\installer\stage_full\maya\conf\french
copy ..\psd2m_maya_plugin\conf\language_english.ini ..\builds\installer\stage_full\maya\conf\english\language.ini
copy ..\psd2m_maya_plugin\conf\language_french.ini ..\builds\installer\stage_full\maya\conf\french\language.ini


@echo Build Maya plugin, Lite version ...
SET _CL_=/UPSDTO3D_FULL_VERSION /O2 /Ob2 /Ot

@rem mkdir ..\builds\plugin\RelWithDebInfo_2023_Lite
mkdir ..\builds\installer\stage_lite\maya\2023\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2023 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2023 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2023\PSDto3D_Maya2023_dev.mll ..\builds\installer\stage_lite\maya\2023\PSDto3D_Maya2023_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2022_Lite
mkdir ..\builds\installer\stage_lite\maya\2022\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2022 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2022 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2022\PSDto3D_Maya2022_dev.mll ..\builds\installer\stage_lite\maya\2022\PSDto3D_Maya2022_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2020_Lite
mkdir ..\builds\installer\stage_lite\maya\2020\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2020 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2020 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2020\PSDto3D_Maya2020_dev.mll ..\builds\installer\stage_lite\maya\2020\PSDto3D_Maya2020_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2019_Lite
mkdir ..\builds\installer\stage_lite\maya\2019\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2019 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2019 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2019\PSDto3D_Maya2019_dev.mll ..\builds\installer\stage_lite\maya\2019\PSDto3D_Maya2019_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2018_Lite
mkdir ..\builds\installer\stage_lite\maya\2018\
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2018 /project psd2m_maya_plugin
devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2018 /project psd2m_maya_plugin
copy ..\builds\plugin\RelWithDebInfo_Maya2018\PSDto3D_Maya2018_dev.mll ..\builds\installer\stage_lite\maya\2018\PSDto3D_Maya2018_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2017_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2017\
@rem devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /clean RelWithDebInfo_Maya2017 /project psd2m_maya_plugin
@rem devenv ..\psd2m_maya_plugin\projects\PSD23D.sln /rebuild RelWithDebInfo_Maya2017 /project psd2m_maya_plugin
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2017\PSDto3D_Maya2017_dev.mll ..\builds\installer\stage_lite\maya\2017\PSDto3D_Maya2017_%PLUGIN_VER%.mll

@echo Staging files, Lite version ...
xcopy ..\builds\photoshop ..\Builds\installer\stage_lite\photoshop\ /E /Y
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2023\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2023\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2022\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2022\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2020\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2020\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2019\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2019\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2018\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2018\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_maya_plugin\shelf\Maya2017\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2017\shelf_PSDto3D.mel

mkdir ..\builds\installer\stage_lite\maya\icons
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" "..\builds\installer\stage_lite\maya\icons\PSD to 3D Icons.ico"
copy "..\psd2m_maya_plugin\icons\PSD to 3D Icons.ico" ".\TEMP_PSDto3D_INSTALLER.ico"

mkdir ..\builds\installer\stage_lite\docs
copy "..\psd2m_maya_plugin\docs\Online Documentation.url" "..\builds\installer\stage_lite\docs\Online Documentation.url"

mkdir ..\builds\installer\stage_lite\maya\conf\english
mkdir ..\builds\installer\stage_lite\maya\conf\french
copy ..\psd2m_maya_plugin\conf\language_english.ini ..\builds\installer\stage_lite\maya\conf\english\language.ini
copy ..\psd2m_maya_plugin\conf\language_french.ini ..\builds\installer\stage_lite\maya\conf\french\language.ini

mkdir ..\builds\installer\output
mkdir ..\builds\installer\installers

@echo Building installer, Pro version
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ./psd2m_full.iss ./TEMP_psd2m_full_%PLUGIN_VER%.iss
.\InnoSetup602\compil32.exe /cc ./TEMP_psd2m_full_%PLUGIN_VER%.iss
move .\Output\setup.exe ..\builds\installer\installers\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_pro_setup.exe

@echo Building installer, Lite version
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ./psd2m_lite.iss ./TEMP_psd2m_lite_%PLUGIN_VER%.iss
.\InnoSetup602\compil32.exe /cc ./TEMP_psd2m_lite_%PLUGIN_VER%.iss
move .\Output\setup.exe ..\builds\installer\installers\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_lite_setup.exe

del ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_3D_bin_password_edfilms.7z
"C:\Program Files\7-zip\7z.exe" a -pedfilms -r ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_bin_password_edfilms.7z ..\builds\installer\stage* ..\builds\installer\installer*

echo %date%  - %time%
