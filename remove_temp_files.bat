rmdir /S /Q projects\.vs
rmdir /S /Q Builds\int
rmdir /S /Q Builds\lib
rmdir /S /Q psd2m_unreal\.vs
rmdir /S /Q psd2m_unreal\DerivedDataCache
rmdir /S /Q psd2m_unreal\Intermediate
rmdir /S /Q psd2m_unreal\Plugins\Editor\PSDtoUnreal\Intermediate
rmdir /S /Q psd2m_unreal\Plugins\Editor\PSDtoUnreal\Binaries\ThirdParty\PSDto3DLibrary\Win64
del /S /Q /F psd2m_unreal\Binaries\*.pdb
del /S /Q /F psd2m_unreal\Plugins\Editor\PSDtoUnreal\Binaries\*.pdb