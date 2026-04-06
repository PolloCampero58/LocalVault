// Copyright (c) 2026 GregOrigin. All Rights Reserved.

using UnrealBuildTool;

public class LocalVault : ModuleRules
{
	public LocalVault(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
		new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UnrealEd",
			"ToolMenus",
			"Projects",
			"InputCore",
			"EditorStyle",
			"WorkspaceMenuStructure",
			"DeveloperSettings",
			"Json",
			"JsonUtilities",
			"HTTP",
			"EditorSubsystem",
			"Fab"
		}
		);	}
}