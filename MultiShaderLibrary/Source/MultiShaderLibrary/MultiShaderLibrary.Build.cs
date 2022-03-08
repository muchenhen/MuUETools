// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MultiShaderLibrary : ModuleRules
{
	public MultiShaderLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Core",
				"Json",
				"ContentBrowser",
				"SandboxFile",
				"TargetPlatform",
				"DesktopPlatform",
				"Projects",
				"Settings",
				"RHI",
				"EngineSettings",
				"AssetRegistry",
				"PakFileUtilities",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UnrealEd",
				"Projects",
				"DesktopPlatform",
				"InputCore",
				"EditorStyle",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RenderCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
