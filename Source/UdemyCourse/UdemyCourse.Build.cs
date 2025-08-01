// Copyright MODogma. All Rights Reserved.

using UnrealBuildTool;

public class UdemyCourse : ModuleRules
{
	public UdemyCourse(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				//System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Public",

            }
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Blutility",
				"EditorScriptingUtilities",
				"Niagara",
				"UMG",
				"UnrealEd",
				"AssetRegistry",
				"InputCore",
				"Projects",
				"AssetTools",
				"SceneOutliner",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"MaterialEditor",
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
