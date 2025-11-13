// Copyright (c) 2024 Lorenzo Santa Cruz. All rights reserved.

using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
	public class ArsInstancedActorsTestSuite : ModuleRules
	{
		public ArsInstancedActorsTestSuite(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"ArsInstancedActors",
					"AITestSuite",
				}
			);
		}
	}
}