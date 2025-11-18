// Copyright TensorWorks Pty Ltd. All Rights Reserved.

using UnrealBuildTool;

public class Buccaneer4PixelStreaming2 : ModuleRules
{
	public Buccaneer4PixelStreaming2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"PixelStreaming2",
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
				"BuccaneerCommon",
				"BuccaneerStats"
			});
	}
}
