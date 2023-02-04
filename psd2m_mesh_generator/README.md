<a id="Top"></a> 
# Summary 
* [Getting Started](#Getting_Started)
  * [Project title](#Project_title)
  * [Features](#Features)
* [Built With](#Built_With)
  * [How to build](#How_to_build)
  * [How to use](#How_to_use_the_plugin)
* [Project structure](#Project_structure)
  * [How to start in the project](#How_to_start_in_the_project)
* [Contibuting](#Contibuting)
* [Versioning](#Versioning)
* [Authors](#Authors)
* [Licence](#Licence)

<a id="Project_title"></a>
# Project title
"PSD2M mesh generator" is a part developed and used in a maya plugin for generate the mesh based on the data extract from the psd. This library allow to read the information of a psd.

<a id="Features"></a>
# Features
- Convert bezier curve.
- Define bounding box of the layer content.
- Generate the base structure of a mesh from a generated grid.

<a id="Built_With"></a>
# Built With
* [CMake](https://cmake.org/download/) - solution generator
* [Visual studio 2017](https://visualstudio.microsoft.com/fr/downloads/) - IDE 

<a id="How_to_Build"></a>
## How to build

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

### Compile with Visual studio:
- Right click on the project and select "build".
- The default path for the build generation is in a "build" folder at the same level of the project directory.

<a id="How_to_use"></a>
## How to use
the mesh generator take the contour bezier curve, a precision and the bounding box in input and return an array with the vertices and the polygon definition. The public access is the method "generateMeshData" in the "mesh.h" header.
<a id="Project_structure"></a>
# Project structure
- You can create your curve with the bezierCurve script.
- You can determine the bounding box of your curves with the boundingBox script.
- You can feed the mesh script of the two previous information to generate the data of the mesh object.

<a id="How_to_start_in_the_project"></a>
## How to start in the project
- the mesh.h header is the core of this part of the project.

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