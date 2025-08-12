// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Buccaneer4PixelStreaming : ModuleRules
{
	public Buccaneer4PixelStreaming(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PixelStreaming",
				"Json",
				"CoreUObject",
				"DeveloperSettings",
				"EngineSettings"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"BuccaneerCommon"
			});
	}
}
