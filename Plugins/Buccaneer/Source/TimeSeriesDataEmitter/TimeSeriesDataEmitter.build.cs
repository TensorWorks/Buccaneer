// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TimeSeriesDataEmitter : ModuleRules
{
	public TimeSeriesDataEmitter(ReadOnlyTargetRules Target) : base(Target)
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
			"SemanticEventEmitter",
			"Json"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{	
			"BuccaneerCommon",
		});

		PublicDefinitions.Add("_USE_32BIT_TIME_T");
	}
}
