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
	void ConsumeStat(FPixelStreamingPlayerId PlayerId, FName StatName, float StatValue);

	IConsoleVariable* CVarBuccaneer4PixelStreamingEnableStats;
	
private:

	double LoggingStart;
	double ReportingInterval;

    TMap<FString, FString> StatDescriptionMap;

    TSharedPtr<FJsonObject> JsonObject;
};
