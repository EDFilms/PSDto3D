// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class PSDto3DLibrary : ModuleRules
{
	public PSDto3DLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/Public");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			//PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "PSDto3D_Standalone.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			// Don't use this.  Plugin loads module explicitly, and will fail if module previously loaded by Unreal engine
			//PublicDelayLoadDLLs.Add("PSDto3D_Standalone.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/PSDto3D_Standalone.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/Qt5Core.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/Qt5Gui.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/Qt5Widgets.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/imageformats/qgif.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/imageformats/qico.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/imageformats/qjpeg.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/platforms/dqirect2D.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/platforms/qminimal.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/platforms/qoffscreen.dll");
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/PSDto3DLibrary/Win64/platforms/qwindows.dll");
		}
	}
}
