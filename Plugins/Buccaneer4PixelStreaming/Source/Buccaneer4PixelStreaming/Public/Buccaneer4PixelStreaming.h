// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BuccaneerCommon.h"
#include "Modules/ModuleManager.h"
#include "PixelStreamingDelegates.h"
#include "IPixelStreamingModule.h"

#include "Dom/JsonObject.h"
#include "PixelStreamingPlayerId.h"

DECLARE_LOG_CATEGORY_EXTERN(BuccaneerPixelStreaming, Log, All);


class FBuccaneer4PixelStreamingModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void Setup();
	UFUNCTION()
	void HandlePlayerDisconnect(FPixelStreamingPlayerId PlayerId, bool WasQualityController);
	void ConsumeStat(FPixelStreamingPlayerId PlayerId, FName StatName, float StatValue);
	
private:
	void OnStreamerReady(IPixelStreamingModule& Module);
	void RegisterMetadata(FString Key, FString Value);
	void RegisterMetric(FString Name, FString Description, FString Type);

	double LoggingStart;
	double ReportingInterval;

	FBuccaneerCommonModule* BuccaneerCommonModule;

	TSharedPtr<FJsonObject> JsonObject; 
	TSharedPtr<FJsonObject> SetupJson; 
    TSharedPtr<FJsonObject> MetricJson;

	bool bEnableStats = true;
};
