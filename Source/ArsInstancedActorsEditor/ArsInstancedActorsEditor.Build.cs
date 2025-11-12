// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class ArsInstancedActorsEditor : ModuleRules
	{
		public ArsInstancedActorsEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
				}
			);

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"UnrealEd",
					"LevelEditor",
					"SlateCore",
					"Slate",
					"LevelEditor",
					"ArsInstancedActors",
					"MassEntity"
				}
			);
		}
	}
}
