echo %date%  - %time%

set BUILD_DATE=%date%
set BUILD_VER=077
set PLUGIN_VER_SHORT=165
set PLUGIN_VER_BASE=1.6.5
set PLUGIN_VER=%PLUGIN_VER_BASE%.%BUILD_VER%
set PLUGIN_YEAR=2024

@echo Updating file property resources ...
copy ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc.BACKUP
findreplace PLUGIN_DESC_TOKEN "PSDto3D Version %PLUGIN_VER_BASE%, Build%BUILD_VER%, Date %BUILD_DATE%" ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_YEAR_TOKEN %PLUGIN_YEAR% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\src\psd2m_core.rc ..\psd2m_core\src\psd2m_core.rc

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

@rem mkdir ..\builds\plugin\RelWithDebInfo_2025_Full
mkdir ..\builds\installer\stage_full\maya\2025\
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2025 /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2025 /project psd2m_core
copy ..\builds\plugin\RelWithDebInfo_Maya2025\PSDto3D_Maya2025_dev.mll ..\builds\installer\stage_full\maya\2025\PSDto3D_Maya2025_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2024_Full
mkdir ..\builds\installer\stage_full\maya\2024\
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2024 /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2024 /project psd2m_core
copy ..\builds\plugin\RelWithDebInfo_Maya2024\PSDto3D_Maya2024_dev.mll ..\builds\installer\stage_full\maya\2024\PSDto3D_Maya2024_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2023_Full
mkdir ..\builds\installer\stage_full\maya\2023\
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2023 /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2023 /project psd2m_core
copy ..\builds\plugin\RelWithDebInfo_Maya2023\PSDto3D_Maya2023_dev.mll ..\builds\installer\stage_full\maya\2023\PSDto3D_Maya2023_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2022_Full
mkdir ..\builds\installer\stage_full\maya\2022\
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2022 /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2022 /project psd2m_core
copy ..\builds\plugin\RelWithDebInfo_Maya2022\PSDto3D_Maya2022_dev.mll ..\builds\installer\stage_full\maya\2022\PSDto3D_Maya2022_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2020_Full
mkdir ..\builds\installer\stage_full\maya\2020\
devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2020 /project psd2m_core
devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2020 /project psd2m_core
copy ..\builds\plugin\RelWithDebInfo_Maya2020\PSDto3D_Maya2020_dev.mll ..\builds\installer\stage_full\maya\2020\PSDto3D_Maya2020_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2019_Full
@rem mkdir ..\builds\installer\stage_full\maya\2019\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2019 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2019 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2019\PSDto3D_Maya2019_dev.mll ..\builds\installer\stage_full\maya\2019\PSDto3D_Maya2019_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2018_Full
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2018 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2018 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2018\PSDto3D_Maya2018_dev.mll ..\builds\installer\stage_full\maya\2018\PSDto3D_Maya2018_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2017_Full
@rem mkdir ..\builds\installer\stage_full\maya\2017\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2017 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2017 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2017\PSDto3D_Maya2017_dev.mll ..\builds\installer\stage_full\maya\2017\PSDto3D_Maya2017_%PLUGIN_VER%.mll

@echo Staging files, Pro version ...
xcopy ..\builds\photoshop ..\Builds\installer\stage_full\photoshop\ /E /Y
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2025\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2025\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2024\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2024\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2023\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2023\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2022\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2022\shelf_PSDto3D.mel
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2020\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2020\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2019\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2019\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2018\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2018\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2017\shelf_PSDto3D.mel ..\builds\installer\stage_full\maya\2017\shelf_PSDto3D.mel

mkdir ..\builds\installer\stage_full\maya\icons
copy "..\psd2m_core\icons\PSD to 3D Icons.ico" "..\builds\installer\stage_full\maya\icons\PSD to 3D Icons.ico"
copy "..\psd2m_core\icons\PSD to 3D Icons.ico" ".\TEMP_PSDto3D_INSTALLER.ico"

mkdir ..\builds\installer\stage_full\docs
copy "..\psd2m_core\docs\Online Documentation.url" "..\builds\installer\stage_full\docs\Online Documentation.url"

mkdir ..\builds\installer\stage_full\maya\conf\english
mkdir ..\builds\installer\stage_full\maya\conf\french
copy ..\psd2m_core\conf\language_english.ini ..\builds\installer\stage_full\maya\conf\english\language.ini
copy ..\psd2m_core\conf\language_french.ini ..\builds\installer\stage_full\maya\conf\french\language.ini


@rem No longer supporting Lite version, as software is now open-source
@rem @echo Build Maya plugin, Lite version ...
@rem SET _CL_=/UPSDTO3D_FULL_VERSION /O2 /Ob2 /Ot

@rem @rem mkdir ..\builds\plugin\RelWithDebInfo_2025_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2025\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2025 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2025 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2025\PSDto3D_Maya2025_dev.mll ..\builds\installer\stage_lite\maya\2025\PSDto3D_Maya2025_%PLUGIN_VER%.mll

@rem mkdir ..\builds\installer\stage_lite\maya\2024\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2024 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2024 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2024\PSDto3D_Maya2024_dev.mll ..\builds\installer\stage_lite\maya\2024\PSDto3D_Maya2024_%PLUGIN_VER%.mll

@rem mkdir ..\builds\installer\stage_lite\maya\2023\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2023 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2023 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2023\PSDto3D_Maya2023_dev.mll ..\builds\installer\stage_lite\maya\2023\PSDto3D_Maya2023_%PLUGIN_VER%.mll

@rem @rem mkdir ..\builds\plugin\RelWithDebInfo_2022_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2022\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2022 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2022 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2022\PSDto3D_Maya2022_dev.mll ..\builds\installer\stage_lite\maya\2022\PSDto3D_Maya2022_%PLUGIN_VER%.mll

@rem @rem mkdir ..\builds\plugin\RelWithDebInfo_2020_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2020\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2020 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2020 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2020\PSDto3D_Maya2020_dev.mll ..\builds\installer\stage_lite\maya\2020\PSDto3D_Maya2020_%PLUGIN_VER%.mll

@rem @rem mkdir ..\builds\plugin\RelWithDebInfo_2019_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2019\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2019 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2019 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2019\PSDto3D_Maya2019_dev.mll ..\builds\installer\stage_lite\maya\2019\PSDto3D_Maya2019_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2018_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2018\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2018 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2018 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2018\PSDto3D_Maya2018_dev.mll ..\builds\installer\stage_lite\maya\2018\PSDto3D_Maya2018_%PLUGIN_VER%.mll

@rem mkdir ..\builds\plugin\RelWithDebInfo_2017_Lite
@rem mkdir ..\builds\installer\stage_lite\maya\2017\
@rem devenv ..\projects\PSDto3D.sln /clean RelWithDebInfo_Maya2017 /project psd2m_core
@rem devenv ..\projects\PSDto3D.sln /rebuild RelWithDebInfo_Maya2017 /project psd2m_core
@rem copy ..\builds\plugin\RelWithDebInfo_Maya2017\PSDto3D_Maya2017_dev.mll ..\builds\installer\stage_lite\maya\2017\PSDto3D_Maya2017_%PLUGIN_VER%.mll

@rem @echo Staging files, Lite version ...
@rem xcopy ..\builds\photoshop ..\Builds\installer\stage_lite\photoshop\ /E /Y
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2025\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2025\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2024\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2024\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2023\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2023\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2022\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2022\shelf_PSDto3D.mel
@rem @rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2020\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2020\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2019\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2019\shelf_PSDto3D.mel
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2018\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2018\shelf_PSDto3D.mel
@rem @rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ..\psd2m_core\shelf\Maya2017\shelf_PSDto3D.mel ..\builds\installer\stage_lite\maya\2017\shelf_PSDto3D.mel

@rem mkdir ..\builds\installer\stage_lite\maya\icons
@rem copy "..\psd2m_core\icons\PSD to 3D Icons.ico" "..\builds\installer\stage_lite\maya\icons\PSD to 3D Icons.ico"
@rem copy "..\psd2m_core\icons\PSD to 3D Icons.ico" ".\TEMP_PSDto3D_INSTALLER.ico"

@rem mkdir ..\builds\installer\stage_lite\docs
@rem copy "..\psd2m_core\docs\Online Documentation.url" "..\builds\installer\stage_lite\docs\Online Documentation.url"

@rem mkdir ..\builds\installer\stage_lite\maya\conf\english
@rem mkdir ..\builds\installer\stage_lite\maya\conf\french
@rem copy ..\psd2m_core\conf\language_english.ini ..\builds\installer\stage_lite\maya\conf\english\language.ini
@rem copy ..\psd2m_core\conf\language_french.ini ..\builds\installer\stage_lite\maya\conf\french\language.ini

mkdir ..\builds\installer\output
mkdir ..\builds\installer\installers

@echo Building installer, Pro version
findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ./psd2m_full.iss ./TEMP_psd2m_full_%PLUGIN_VER%.iss
.\InnoSetup602\compil32.exe /cc ./TEMP_psd2m_full_%PLUGIN_VER%.iss
move .\Output\setup.exe ..\builds\installer\installers\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_pro_setup.exe

@rem @echo Building installer, Lite version
@rem findreplace PLUGIN_VER_TOKEN %PLUGIN_VER% ./psd2m_lite.iss ./TEMP_psd2m_lite_%PLUGIN_VER%.iss
@rem .\InnoSetup602\compil32.exe /cc ./TEMP_psd2m_lite_%PLUGIN_VER%.iss
@rem move .\Output\setup.exe ..\builds\installer\installers\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_lite_setup.exe

@echo Deleting temp files ...
del ..\psd2m_core\src\psd2m_core.rc
move ..\psd2m_core\src\psd2m_core.rc.BACKUP ..\psd2m_core\src\psd2m_core.rc
del ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_3D_bin_password_edfilms.7z
"C:\Program Files\7-zip\7z.exe" a -pedfilms -r ..\builds\installer\Build%BUILD_VER%-V%PLUGIN_VER_SHORT%_PSD_to_Maya_bin_password_edfilms.7z ..\builds\installer\stage* ..\builds\installer\installer*

echo %date%  - %time%
