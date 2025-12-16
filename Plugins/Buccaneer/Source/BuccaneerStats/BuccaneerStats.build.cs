// Copyright TensorWorks Pty Ltd. All Rights Reserved.

using UnrealBuildTool;

public class BuccaneerStats : ModuleRules
{
	public BuccaneerStats(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{	
			"Core",
			"RHI",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"RenderCore",
			"BuccaneerEvents",
			"Json"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{	
			"BuccaneerCommon",
		});

		PublicDefinitions.Add("_USE_32BIT_TIME_T");
	}
}
