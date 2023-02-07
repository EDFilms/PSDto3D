mkdir ..\..\Builds\photoshop\cc\
ZXPSignCmd_win64 -sign .\Extension ..\..\Builds\photoshop\cc\PsdExporter.zxp .\key_edfilms.p12 edfilms2018 
copy install_plugin.bat ..\..\Builds\photoshop\cc
xcopy ..\install_tool ..\..\Builds\photoshop\cc\ /E /Y

mkdir ..\..\Builds\photoshop\cc\Extension
@rem need to copy additional signing files created by ZXPSignCmd_win64, so an xcopy of the source files isn't good enough
@rem xcopy .\Extension ..\..\Builds\photoshop\cc\Extension /E /Y
@rem instead extract the zxp file
"C:\Program Files\7-zip\7z.exe" x -o..\..\Builds\photoshop\cc\Extension -r ..\..\Builds\photoshop\cc\PsdExporter.zxp 
