@rem Don't build an extension, not supported in CS6 until UI is converted from HTML to Flash
@rem Instead, just copy and rename the .jsx file
mkdir ..\..\Builds\photoshop\cs6\
copy .\PsdExporter\jsx\PsdExporter.jsx ..\..\Builds\photoshop\cs6\PsdExporter_CS6.jsx


@rem pushd ucf
@rem java -jar ucf.jar -package -storetype PKCS12 -keystore key_edfilms.p12 -storepass edfilms2018 PsdExporter.zxp -C ..\PsdExporter .\
@rem move PsdExporter.zxp .. 
@rem popd