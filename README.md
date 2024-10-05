PSD to 3D
=============

Welcome to the PSD to 3D source code!

With the code in this repository, you can build **PSD to Maya** and **PSD to FBX** for Windows. Modify the code in any way you can imagine, and share your changes with others!

We have [official documentation](https://https://www.edfilms.net/) available for these tools. If you're looking for the answer to something, you may want to start in one of these places:

*   [User guide for PSD to 3D](https://www.edfilms.net/)
*   [Tutorials and artwork tips](https://vimeo.com/danielgies/)
*   [Discord channel](https://discord.com/)

If you need more, just ask! Developers are invited to join our [Open Source community](https://discord.com/), and we're proud to be part of a well-meaning, friendly, and welcoming community of thousands.


Branches
--------

We publish source for these tools in several ways:

*   The lastest stable release is published through [GitHub releases](https://github.com/EDFilms/PSDto3D/releases).  Check there for tested releases of source code, including pre-build binaries and installers for **PSD to Maya** and **PSD to FBX**, all in a single bundle.

*   Most active development on PSD to 3D happens in [this](https://github.com/EDFilms/PSDto3D/tree/michaelson) branch. It reflects the cutting edge of the tools and may be buggy — it may not even compile. We make it available for battle-hardened developers eager to test new features or work in lock-step with us.

    If you choose to work in this branch, be aware that it is likely to be **ahead** of the branches for the current official release and the next upcoming release. Therefore, setups and configuration files created by these tools may not be compatible with public releases until we create a new official release.

*   Numbered builds and official public releases are usually merged to the [main](https://github.com/EDFilms/PSDto3D/tree/main) and [devel](https://github.com/EDFilms/PSDto3D/tree/devel) branches, as a single commit each.  So, any one commit on either of these corresponds to a tested, numbered release.

Other short-lived branches may pop-up from time to time as we stabilize new releases or hotfixes.

Getting up and running
----------------------

The steps below take you through cloning your own private fork, then compiling and running the tools yourself:

### Windows

1.  Install **[GitHub Desktop for Windows](https://desktop.github.com/)** then **[fork and clone our repository](https://guides.github.com/activities/forking/)**. 

    When GitHub Desktop is finished creating the fork, it asks, **How are you planning to use this fork?**. Select **For my own purposes**. This will help ensure that any changes you make stay local to your repository and avoid creating unintended pull requests. You'll still be able to make pull requests if you want to submit modifications that you make in your fork back to our repository.

    Other options:

    -   To use Git from the command line, see the [Setting up Git](https://help.github.com/articles/set-up-git/) and [Fork a Repo](https://help.github.com/articles/fork-a-repo/) articles.

    -   If you'd prefer not to use Git, you can get the source with the **Download ZIP** button on the right. Note that the zip utility built in to Windows marks the contents of .zip files downloaded from the Internet as unsafe to execute, so right-click the .zip file and select **Properties…** and **Unblock** before decompressing it.

1.  Install **[Visual Studio for Windows](https://visualstudio.microsoft.com/vs/older-downloads/)**

    -   PSDtoMaya matches Maya's development environment.  Each version of Maya is native to a specific version of Visual Studio.  Plug-ins for older versions of Maya can be build with newer version of Visual Studio, if corresponding MSVC build tools (also called Platform Toolset) and Windows SDK are installed.  These can be selected under **Individual Components** during install.  The list below shows which versions are required for each.  Alternately, check `\devkitBase\devkit\README.md` in the Maya devkit.
        -   Maya 2020:  Built with **VS2017** which is **MSVC v141**, using Windows SDK **10.0.10586** and **Qt 5.15.2**

        -   Maya 2022 and 2023:  Built with **VS2019** which **MSVC v142**, using Windows SDK **10.0.18362.0** and **Qt 5.15.2**

        -   Maya 2024:  Built with **VS2022** which is **MSVC v143**, using Windows SDK **10.0.22621.0** and **Qt 5.15.2.0**

        -   Maya 2025:  Built with **VS2022** which is **MSVC v143**, using Windows SDK **10.0.22621.0** and **Qt 6.5.3.0**
        
        -   PSD to FBX:  Built with **VS2019** which **MSVC v142**, using Windows SDK **10.0.18362.0** and **Qt 5.15.2**.

    -   Install **Visual Studio 2017** to support Maya 2020.  Add the following options:
    
        *VC++ 2017 version 15.9 v14.16 Libs for Spectre (x86 and x64)*
    
        *Windows 10 SDK (10.0.10586.0)*.  Note this component is not available from VS2019 or VS2022 installers

    -   Install **Visual Studio 2019** to support Maya 2020-2023 and PSD to FBX.  Add the following options:
    
        *MSVC v142 - VS 2019 C++ x64/x86 Spectre-mitigiated libs (latest)*
    
        *Windows 10 SDK (10.0.18362)*

    -   Install **Visual Studio 2022** to support Maya 2024-2025.  Add the following options:
    
        *MSVC v143 - VS 2022 C++ x64/x86 Spectre-mitigiated libs (latest)*
    
        *Windows 11 SDK (10.0.22621)*
    
    -   Note that **older Visual Studio versions should be installed before newer versions**.

    -   To create a **PSD to Maya installer**, the plug-in will be built for **all versions** of Maya.  You should install all components above.  Note the VS2022 installer provides all components except *Windows 10 SDK (10.0.10586.0)*, so VS2017 and VS2019 are optional otherwise.


### PSD to Maya

1.  Consider which versions of Maya which you intend to target.  If you want to create an installer package, then **all versions** of Maya must be installed.  As of September 2024, the plug-in supports: 

    - Maya 2020
    - Maya 2022
    - Maya 2023
    - Maya 2024
    - Maya 2025

1.  Prepare the Maya Devkit for each Maya version.  Start by downloading each devkit package

    -   Find the devkit at https://aps.autodesk.com/developer/overview/maya.  The download should be a zipfile named like `Autodesk_Maya_2025_DEVKIT_Windows.zip`.  Only a base version package is needed, not Update 1 or Update 2 etc, since updates maintain plug-in compatibility.

1.  Create the devkit directories
    -   Create directory `C:\build`.

    -   Create subfolders for each Maya version, named like `C:\build\Maya2025`.  These are the devkit root directories for each build.
    
    -   These root directory paths are **hard-coded** in the project file.  To redirect this, open the project file `\projects\psd2m_core\psd2m_core.vcxproj` in a text editor and replace each pathname like `C:\build\Maya2025`.

1.  Populate the devkit directories
    -   Each devkit zipfile has a base directory named `devkitBase`.  Underneath are several subfolders. Copy those subfolders to the respective `C:\build` root locations.  This should create directories like `C:\build\Maya2025\include`, `C:\build\Maya2025\lib`, etc.

1. **Patch** each devkit

    -   **include** directory, named like `C:\build\Maya2025\include`
    
        For Maya 2020-2024, this contains a Qt zipfile like `qt_6.5.3_vc14-include.zip`.  Unzip it into here.
        
        For Maya 2025, the devkit contains `\devkitBase\Qt.zip` which contains `\Qt\include`. Unzip it here.

    -   **lib** directory, named like `C:\build\Maya2025\lib`
    
        For Maya 2020-2024, this should already contain the necessary files.

        For Maya 2025, the devkit contains `\devkitBase\Qt.zip` which contains `\Qt\lib`. Unzip it here.

    -   **mkspecs** directory, named like `C:\build\Maya2025\mkspecs`
    
        For Maya 2020-2024, this contains a Qt zipfile like `qt_5.15.2_vc14-mkspecs.zip`.  Unzip it here.

        For Maya 2025, the devkit contains `\devkitBase\Qt.zip` which contains `\Qt\mkspecs`. Unzip it here.

    -   **Bin** directory, named like `C:\build\Maya2025\bin`
    
        For Maya 2020-2024, the devkit contains `\devkitBase\devkit\bin`.  Copy those files here.

        For Maya 2025, the devkit contains `\devkitBase\Qt.zip` which contains `\Qt\bin`.  Unzip it here.

1.  From your source folder, open `\projects\PSDto3D.sln` in Visual Studio.  Set the solution configuration to match your Maya version, for example `RelWithDebInfo_Maya2025`.  The prefix indicates a release build with debug symbols.  Generally PSD to Maya should **not** be run in debug mode.  Maya itself runs in release mode and can potentially conflict with a plug-in linked against debug runtimes DLLs.

1.  In the Solution Explorer, right click the `psd2m_core` project and select Build.  If all goes well, you now have a Maya plug-in located under the `\Builds\plugin` directory.  It should be named like `\Builds\plugin\RelWithDebInfo_Maya2025\PSDto3D_Maya2025_dev.mll`, where `.mll` indicates a Maya plug-in DLL.

    -   If you have made any code changes, the Maya 2025 version of the plug-in may produce compile errors not present in other versions.  This is because it uses Qt6 which disables the `/permissive` setting, and forces all code to strictly follow the language standards.  Qt6 headers use preprocesor commands to toggle this on, will not compile if toggled off with `/permissive-`.

1.  Copy the plug-in and shelf file to Maya, and patch the shelf file
    -   Copy the `.mll` to the Maya user plug-ins directory, under your `Documents` folder, like `C:\Users\YourUserName\Documents\maya\2025\plug-ins`.  It may be necessary to manually create the `plug-ins` directory.
    -   Copy the shelf file `shelf_PSDto3D.mel` from `\psd2m_core\shelf` to the Maya user shelves folder.  For example, copy `\psd2m_core\shelf\Maya2025\shelf_PSDto3D.mel` to `C:\Users\`**`YourUserName`**`\Documents\maya\2025\prefs\shelves`
    -   **Patch** the new shelf file from the previous step.  In a text editor, replace the string `PSDto3D_Maya2025_PLUGIN_VER_TOKEN.mll`.  This appears in two places.  Swap in the plug-in `.mll` filename, like `PSDto3D_Maya2025_dev.mll`.
    -   Alternately, you may replace the pathname in the shelf file with the absolute path to the development binary.  Remove the `$shelfDir +` before the path, keeping the escaped `\"` before and after the path.  For example, replace `$shelfDir + \"../../plug-ins/PSDto3D_Maya2025_PLUGIN_VER_TOKEN.mll\"` with `\"C:/`**`YourRepository`**`/Builds/plugin/RelWithDebInfo_Maya2025/PSDto3D_Maya2025_dev.mll\"`.  Change **all backslashes to forward slashes**, or escape the backslashes as `\\`.  Using the absolute pathname allows you to rebuild the plug-in without copying files again.

1. Launch Maya.  It should now recognize the plug-in, but doesn't load it yet.  There should be a shelf named *PSDto3D*.  Click the first button in this shelf.  This loads the plug-in, and launches the main window.  A new menu named *PSDto3D* appears, which can anternately be used the launch the main window
    -   If the plug-in is located in the Maya user plug-in directory, you can toggle it to load automatically from the Plug-in Manager.  Launch it with `Windows menu->Settings/Preferences->Plug-in Manager`.  Look for the PSD to 3D `.mll` file and check Auto Load.  This will cause the *PSDto3D* menu to appear when Maya launches.


### PSD to FBX

1.  Configure the [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk).

    -   Download FBX SDK 2020.3.7 for **Visual Studio 2019**.  The package is an `.exe` but can be extracted as a zipfile using [7-zip](https://7-zip.org/).

    -   Create directory `C:\build\fbx`, and unzip the SDK there.  This should create directories `C:\build\fbx\include` and `C:\build\fbx\lib`

    -   This directory location is **hard-coded** in the project file `projects\psd2m_fbx\psd2m_fbx.vcxproj`.  To redirect this, open the project file in a text editor and replace the pathname `C:\build\fbx`.

1.  Configure Qt 5.15.2

    -   Create directory `C:\build\qt5`

    -   This directory location is **hard-coded** in the project file `projects\psd2m_fbx\psd2m_fbx.vcxproj`.  To redirect this, open the project file in a text editor and replace the pathname `C:\build\qt5`.

    -   **OPTION 1**:  Build Qt following the instructions in **Building Qt** below

    -   **OPTION 2**:  Copy files from an installed copy of Maya, and its devkit.  PSD to FBX is tested with Qt 5.15.2.0 and VS2019, as used by Maya 2022-2023.  Start by creating `C:\build\qt5`, and populate with identical contents to a configured devkit folder, as explained under **PSD to Maya**.  Then, create directory `C:\build\qt5\plugins` and copy the folders `\plugins\platforms` and `\plugins\imageformats` from Maya's root directory.  You should have `C:\build\qt5\plugins\platforms\qwindows.dll` and others.  

1.  From your source folder, open `\projects\PSDto3D.sln` in Visual Studio.  Set the solution configuration to `RelWithDebInfo_FBX`.  The prefix indicates a release build with debug symbols, which improves performance compared to a full debug build.  However, debugging may be hampered as optimizations are enabled, with some variables optimized away.  You may instead use `RelUnoptimized_FBX`.

1.  In the Solution Explorer, right click the `psd2m_fbx` project and select Build.  If all goes well, you now have a PSD to FBX standalone app named `\Builds\plugin\RelWithDebInfo_FBX\PSDto3D_FBX_dev.exe`.  It's ready to run!

1.  You may use **other versions of Visual Studio**, if you modify the build settings, and if you have Qt built with the same version.  The FBX SDK is available for VS2022, VS2019 and VS2017.  You must configure the *Platform Toolset* and *Windows SDK Version* project settings to match the FBX SDK native compiler version.
    
    -   For example, to use the FBX SDK for Visual Studio 2022 with Qt6, the binaries in `C:\build\qt6` must be built with VS2022.  Then, in both the `psd2m_fbx` and `psd2m_core` project settings, change *Platform Toolset* to `Visual Studio 2022 (v142)` and *Windows SDK Version* to `10.0.22621.0`.  Then change the Solution Configuration so each other project uses the *VS2022* variant.  Use a text editor to change `C:\build\qt5` to `C:\build\qt6` in project files `psd2m_core.vcxproj` and `psd2m_fbx.vcxproj`.


### Installers

1.  Installers are produced by using [Inno Setup](https://jrsoftware.org/isinfo.php).  You don't need to download this software, because a lightweight deployment is already included in the source repository.  Installer builds are performed using batch files located in `\psd2m_installer`.  The batches run all the builds needed, patch resources files with version numbers, invoke InnoSetup, and zip the results.

1.  To run any build, first delete or move the contents of the `\Builds` folder.  The folder should be completely empty before any installer build.

1.  Note **if a build script is cancelled** before completing, there will be lingering changes in `\psd2m_core\src\psd2m_core.rc`.  The build script patches this file with version information, then restores the changes after the build.  But if the batch is interrupted, changes are not restored.  Revert such changes before making a commit.

1.  To build **PSD to Maya**:
    -   (*Optional*) Open the build script `\psd2m_installer\build_installer_maya.bat` in a text editor.  Patch the values set for `BUILD_VER`, `PLUGIN_VER_SHORT`, `PLUGIN_VER_BASE` and `PLUGIN_YEAR` to reflect the current build and version number.
    -   Open the *x64 Native Tools Command Prompt for VS 2022*, navigate to `\psd2m_installer`, and run `build_installer_maya.bat`.

1.  To build **PSD to FBX**:
    -   (*Optional*) Open the build script `\psd2m_installer\build_installer_fbx.bat` in a text editor.  Patch the values set for `BUILD_VER`, `PLUGIN_VER_SHORT`, `PLUGIN_VER_BASE` and `PLUGIN_YEAR` to reflect the current build and version number.
    -   Open the *x64 Native Tools Command Prompt for VS 2019*, navigate to `\psd2m_installer`, and run `build_installer_fbx.bat`.

1.  Output is located in `\Builds\installer`.  The binary deployment is under `\stage_full`, while the installer executable is under `\installers`.  There is also a zipfile with both of these for convenience.  Note the zipfile does not include source code, only binaries.

1.  If you want to archive a source bundle, this should be done separately.  Delete the temporary folder `\projects\.vs` to reduce the size of the archive.


### Building Qt

1.  PSDtoFBX is tested with [Qt 5.15.2](https://download.qt.io/archive/qt/5.15/5.15.2/single/) built from source.  Create the following three folders.
    -   `C:\build\qt5` (final output)
    -   `C:\build\qt5_int` (build intermediates)
    -   `C:\build\q5_src` (source code)
    
1. Download and unzip the source with `C:\build\q5_src` as the root folder, so you have `C:\build\qt5_src\configure.bat`.

1.  For faster builds, download [jom](http://download.qt.io/official_releases/jom/jom.zip) and add it to your PATH variable. This lightweight tool replaces nmake and paralellizes the build across multiple CPU cores.

    A full build requires about 30 minutes to complete on a 16-core machine.  The resulting Qt binaries are currently 1-2 GiB.  The source and intermediates are currently 17-18 GiB, which can be deleted after building.

1.  From the Visual Studio Installer, select and install *Windows 10 SDK (10.0.17763.0)* under Individual Components.  Using this Windows SDK version resolves an error during the configure step, with *\_guard_check_icall_*.

1.  The following batch file was used to build.  Run these commands from the *x64 Native Tools Command Prompt for VS 2019*.
    ```
    echo open x64 Native Tools Command Prompt for VS 2019
    echo unzip qt-everywhere-src-5.15.2.zip to C:\build\qt5_src

    echo setup environment to use Windows SDK 10.0.17763.0 (*IMPORTANT* resolves build errors)
    cd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\"
    call vcvarsall.bat amd64 10.0.17763.0

    echo configure before building
    cd C:\build\qt5_int
    call C:\build\qt5_src\configure -prefix C:\build\qt5 -release -force-debug-info -opensource -nomake examples -nomake tests
    set PATH=C:\BUILD\qt5_int\qtbase\bin;%PATH%

    echo run the build
    jom

    echo copy and patch headers into build location (*IMPORTANT* required for PSDto3D builds)
    jom install
    ```

1. Qt6 version 6.5.3 is is not fully tested with PSDtoFBX, but is supported by PSDtoMaya for Maya 2025, using the VS2022 build configuration.  To preview with PSDtoFBX:
    -   Run the Visual Studio Installer for VS2022 and select *C++ ATL for latest v143 build tools (x86 x x64)*  under Individual Components.  This resolves a build error with *atlbase.h* when building Qt6 from source.
    -   Download and unzip [Qt 6.5.3](https://download.qt.io/official_releases/qt/6.5/6.5.3/single/), and build from the *x64 Native Tools Command Prompt for VS 2022*.
    -   Update the PSDto3D project settings and Solution Configuration, setting *Windows SDK Version* to *10.0.22621.0* and *Platform Toolkit* to *Visual Studio 2022 (v143)*.
    -   Note the Qt6 header files contain commands to disable permissive mode.  This forces strict compiler compliance, and can reveal unexpected build errors in your code.


Licensing
---------

Your access to and use of **PSD to 3D** on GitHub is governed by an End User License Agreement (EULA). For the latest terms and conditions, see the license and FAQ on the [E.D. Films Tools page](https://www.edfilms.net/). If you don't agree to the terms in your chosen EULA, as amended from time to time, you are not permitted to access or use **PSD to 3D**.

Contributions
-------------

We welcome contributions to **PSD to 3D** development through [pull requests](https://github.com/edfilms/PSDto3D/pulls/) on GitHub.

We prefer to take pull requests in our active development branches, particularly for new features. For **PSD to 3D**, use the **devel** branch. Please make sure that all new code adheres to our [coding standards](https://www.edfilms.net/).

For more information on the process and expectations, see [the documentation](https://www.edfilms.net/).

All contributions are governed by the terms of your EULA.


Additional Notes
----------------

**PSD to Unreal** is currently experimental, with no build instructions or beta version ready for evaluation.

**PSD to Maya Lite** is a retired variant.  This source code and build process is for the full-featured variant, previously called **PSD to Maya Pro**.

Your private forks of the PSD to 3D code are associated with your GitHub account permissions. If you unsubscribe or switch GitHub user names, you'll need to create a new fork and upload your changes from the fresh copy.
