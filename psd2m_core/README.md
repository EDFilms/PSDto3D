# README for psd2m_maya_plugin version 1.0.5 build 021 - August 01, 2019

<a id="Top"></a> 
# Summary 
* [Getting Started](#Getting_Started)
  * [Project title](#Project_title)
  * [Features](#Features)
* [Built With](#Built_With)
  * [How to build](#How_to_build)
* [Project structure](#Project_structure)
  * [How to start in the project](#How_to_start_in_the_project)
* [Contibuting](#Contibuting)
* [Versioning](#Versioning)
* [Authors](#Authors)
* [Licence](#Licence)

<a id="Project_title"></a>
# Project title
"PSD2M maya plugin" manage in maya the reading of the psd and the generation of the meshes based on the source of photoshop.

<a id="Features"></a>
# Features
- Display an interface to set parameter generation and load the file.
- Manage the maya mesh structure.
- Manage the maya texture and material structure.

<a id="Built_With"></a>
# Built With
* [CMake](https://cmake.org/download/) - solution generator
* [Visual studio 2017](https://visualstudio.microsoft.com/fr/downloads/) - IDE 

<a id="How_to_Build"></a>
## How to build

### Get the project
-  get the project Psd reader and Mesh generator in separate folder and at the same level of the root maya plugin.
-  Sepecify in the setup/CMakeList.txt the name folder associate to the variable TARGET_NAME_PSD_READER and TARGET_NAME_MESH_GENERATOR corresponding to the folder name of the two previous project..

### Generate the solution:
- Open CMake
- Selection the setup directory.
- Specify your destination folder.
- Click on "configure".
- If your destination directory does not exist, accept to create it.
- A window open, specify the generator for this project to "Visual Studio 15 2017 Win64".
- Click on "Finish".
- Click on "Generate"
- Click on "Open project" -> Visual studio will open with the project generated and all configuration set for build a static library.

Take care to refer the good installation folder of maya and the qmake.exe associate to your maya installation.

### Compile with Visual studio:
- Right click on the project and select "build".
- The default path for the build generation is in a "build" folder at the same level of the project directory.

<a id="Project_structure"></a>
# Project structure
- Main script are on the root of the project.
- The Qt interface is in the interface folder.
- The maya mesh generation, texture and material are int he mesh_maya_generator folder.

<a id="How_to_start_in_the_project"></a>
## How to start in the project
- Read the mainWindowCMD that load the plugin in maya.
- the script of plugin controller manage the psd reader, the mesh generator and the maya mesh generation.

<a id="API_Reference"></a>
## API Reference
[WIP]

<a id="Contibuting"></a>
# Contributing
Please read [CONTRIBUTORS.md](CONTRIBUTORS.md).

<a id="Versionning"></a>
# Versioning
We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [this repo](https://gitlab-ee.cdrin.com/1718_26_EDF/psd2m_psd_reader/tree/master) for the code. 

<a id="Authors"></a>
# Authors
See the list of [CONTRIBUTORS.md](CONTRIBUTORS.md) who participated in this project.

<a id="Licence"></a>
# License
This project is licensed under the GNU License - see the [LICENCE.md](LICENCE.md) file for details.

[Back Top](#Top)