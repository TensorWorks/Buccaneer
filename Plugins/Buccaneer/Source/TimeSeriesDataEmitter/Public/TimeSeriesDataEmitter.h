// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "BuccaneerCommon.h"
#include "Tickable.h"
#include "Dom/JsonObject.h"

DECLARE_LOG_CATEGORY_EXTERN(TimeSeriesDataEmitter, Log, All);

class TIMESERIESDATAEMITTER_API FTimeSeriesDataEmitterModule : public IModuleInterface, public FTickableGameObject
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // FTickableGameObject
    bool IsTickableWhenPaused() const override;
    bool IsTickableInEditor() const override;
    void Tick(float DeltaTime) override;
    TStatId GetStatId() const override;

private:
    void PushStatsHTTP();
    void ComputeUsedMemory();
    void UpdateMetric(FString Name, double Value);

    // Time keeping variables
    double LastTickTime = 0.0;
    double InterimStart = 0.0;
    double InterimDuration = 1.0;
    // Rolling average of times recorded during the defined period
    double InterimMeanFrameTime = 0.0;
    double InterimMeanGameThreadTime = 0.0;
    double InterimMeanRenderThreadTime = 0.0;
    double InterimMeanRHIThreadTime = 0.0;
    double InterimMeanGPUTime = 0.0;
    // Memory metrics
    double UsedVirtualMemory = 0.0;
    double UsedPhysicalMemory = 0.0;
    double UsedGPUMemory = 0.0;
    // Metrics to store information about the number of hangs and number of frames recorded during the interim
    // (using an unsigned int as there shouldn't be more than 4.2 million hangs during a time period, and if there is you have bigger problems)
    double InterimHangCount = 0.0;
    uint32 InterimFrameCount = 1;

    // Variable for storing logging URL and logging object
    TSharedPtr<FJsonObject> JsonObject;
    TSharedPtr<FJsonObject> MetricJson;

    TMap<FString, FString> StatDescriptionMap;
};
