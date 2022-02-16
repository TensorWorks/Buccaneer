// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SemanticEventEmitter : ModuleRules
{
	public SemanticEventEmitter(ReadOnlyTargetRules Target) : base(Target)
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
			"HTTP",
			"Json",
			"BuccaneerCommon"
		});
	}
}
