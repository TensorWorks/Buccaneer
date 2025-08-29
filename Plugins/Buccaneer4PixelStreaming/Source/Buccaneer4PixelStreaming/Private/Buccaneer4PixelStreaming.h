// Copyright TensorWorks Pty Ltd. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "BuccaneerMetrics.h"
#include "IBuccaneerCommonModule.h"
#include "IBuccaneer4PixelStreamingModule.h"
#include "IPixelStreamingModule.h"
#include "Modules/ModuleManager.h"
#include "PixelStreamingDelegates.h"
#include "PixelStreamingPlayerId.h"

class FBuccaneer4PixelStreamingModule : public IBuccaneer4PixelStreamingModule
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void ConsumeStat(FPixelStreamingPlayerId PlayerId, FName StatName, float StatValue);
	
private:
	double LoggingStart;
	double ReportingInterval;

    TMap<FPixelStreamingPlayerId, TMap<FString, FBuccaneerMetric>> PlayerMetrics;
};
