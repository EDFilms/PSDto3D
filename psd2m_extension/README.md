<a id="Top"></a> 
# Summary 
* [Getting Started](#Getting_Started)
  * [Project title](#Project_title)
  * [Screenshots](#Screenshots)
  * [Features](#Features)
* [Built With](#Built_With)
  * [Installation](#How_to_build)
  * [How to build](#Installation)
  * [How to use](#How_to_use_the_plugin)
* [Project structure](#Project_structure)
  * [How to start in the project](#How_to_start_in_the_project)
  * [API Reference](#API_Reference)
* [Contibuting](#Contibuting)
* [Versioning](#Versioning)
* [Authors](#Authors)
* [Licence](#Licence) 

<a id="Getting_Started"></a>
# Getting Started

<a id="Project_title"></a>
## Project title
"PSD2M extension" is an extension photoshop plugin project. The plugin allow to create a tab windows. The process format the psd for prepare to PSD2M maya plugin.

<a id="Screenshots"></a> 
## Screenshots
[WIP]

<a id="Features"></a>
## Features
- Rasterize each layer.
- Generate the path corresponding to the border of each layer (alpha)
- Option to export PNG.
- Register a version of PSD ready for maya plugin in the selected folder.

<a id="Built_With"></a>
# Built With

* [Eclipse](http://www.eclipse.org/downloads/) - IDE
* [Extension builder](https://labs.adobe.com/downloads/extensionbuilder3.html) - Template

<a id="Installation"></a>
## Installation
Please read [INSTALL.md](INSTALL.md)

<a id="How_to_Build"></a>
## How to build
- In eclipse go to File -> export.
- Select Adobe Extension Builder 3 ->Application Extension.
- Create or select the certificate key "*.p12".
- Enter the password for this project it is "edf2018" associate to the key "key_edfilms.p12" at the root of the directory.
- Select in the destination path the "build" folder on the root.
- Click on finish.

<a id="How_to_use_the_plugin"></a>
## How to use
- If you want to install the plugin be sure the build folder contains the "PsdExporter.zpx" file.
- Run install_plugin.bat.

If you want to do the job manually, the bash file contain the to command to call and the path to the ExManCmd.exe to manage the extension on windows. 

After the installation:
- Open photoshop cc 2018.
- Go to Windows->Extension->PsdExporter.
- A tab windows open.
- Enjoy.

<a id="Project_structure"></a>
# Project structure
- <b>Install_tool folder</b>: This folder contain the tool used to install an extension.
- <b>PsdExporter folder</b>: It is the folder of the extension project.
- <b>PsdExporter Content folder</b>: See the Getting started guide of adobe [Getting-Started-guides](<https://github.com/Adobe-CEP/Getting-Started-guides>)

<a id="How_to_start_in_the_project"></a>
## How to start in the project
- Open Eclipse
- Open the project from your workspace.
- "index.html" : represent the definition of the interface.
- "ext.js" :  is associate with loading functionalities and the link between interface and process.
- "jsx/Photoshop.jsx" : contain all the script process on layer.

<a id="API_Reference"></a>
## API Reference
[WIP]

<a id="Contibuting"></a>
## Contributing

Please read [CONTRIBUTORS.md](CONTRIBUTORS.md).
<a id="Versionning"></a>
## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [this repo](https://gitlab-ee.cdrin.com/1718_26_EDF/psd2m_extension/tree/master) for the code. 

<a id="Authors"></a>
## Authors

See the list of [CONTRIBUTORS.md](CONTRIBUTORS.md) who participated in this project.

<a id="Licence"></a>
## License

This project is licensed under the GNU License - see the [LICENCE.md](LICENCE.md) file for details.

[Back Top](#Top)
