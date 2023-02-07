:: Uninstall the extension if already isntall
:: Plugin name = ExtensionBundleId in the manifest
ExMan\ExManCmd.exe /remove "com.EDFilms.PsdExporter"

:: Install the extension on CEP (photoshop)
ExMan\ExManCmd.exe /install ".\PsdExporter.zxp"