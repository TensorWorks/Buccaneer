// Copyright TensorWorks Pty Ltd. All Rights Reserved.

using UnrealBuildTool;

public class BuccaneerCommon : ModuleRules
{
	public BuccaneerCommon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{	
			"Core",
			"Engine",
			"Json"
		});

		PublicDependencyModuleNames.AddRange(new string[]{
			"HTTP",
			"CoreUObject",
			"DeveloperSettings",
			"EngineSettings"
		});
	}
}
