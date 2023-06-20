mkdir ..\..\Builds\photoshop\cc\

ZXPSignCmd_win64 -selfSignedCert CA QC EDFilms EDFilms.PsdExporter edfilms2018 ..\..\Builds\key_selfsigned_photoshop.p12
ZXPSignCmd_win64 -sign .\Extension ..\..\Builds\photoshop\cc\PsdExporter.zxp ..\..\Builds\key_selfsigned_photoshop.p12 edfilms2018
@rem may add timestamp option:  -tsa https://timestamp.geotrust.com/ts

copy install_plugin.bat ..\..\Builds\photoshop\cc
xcopy ..\install_tool ..\..\Builds\photoshop\cc\ /E /Y

mkdir ..\..\Builds\photoshop\cc\Extension
@rem need to copy additional signing files created by ZXPSignCmd_win64, so an xcopy of the source files isn't good enough
@rem xcopy .\Extension ..\..\Builds\photoshop\cc\Extension /E /Y
@rem instead extract the zxp file
"C:\Program Files\7-zip\7z.exe" x -o..\..\Builds\photoshop\cc\Extension -r ..\..\Builds\photoshop\cc\PsdExporter.zxp 
