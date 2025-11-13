// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

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
