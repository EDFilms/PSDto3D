@echo Ensure you have "Windows 11 SDK (10.0.22621.0)" and "MSVC v143 - VS 2022 C++ Arm build tool (v14.38 - 17.8)" installed, from Visual Studio Installer
pushd "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
call vcvarsall.bat amd64 10.0.22621.0
popd

call prepare_headers
call prepare_project
call prepare_solution

devenv HostProject.sln
