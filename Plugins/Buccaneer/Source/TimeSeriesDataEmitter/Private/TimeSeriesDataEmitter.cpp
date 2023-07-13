// Copyright Epic Games, Inc. All Rights Reserved.

#include "TimeSeriesDataEmitter.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "SemanticEventEmitter.h"
#include "BuccaneerCommon.h"
#include "Stats/Stats.h"
#include "Stats/StatsData.h"
#define LOCTEXT_NAMESPACE "FTimeSeriesDataEmitterModule"

#define COMPUTE_MEAN(CurrentMean, NewTime, FrameCount) \
    ((FrameCount - 1) * CurrentMean + NewTime) / FrameCount;

DEFINE_LOG_CATEGORY(TimeSeriesDataEmitter);

void FTimeSeriesDataEmitterModule::StartupModule()
{
    MetricJson = MakeShareable(new FJsonObject());
    JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetField(TEXT("metrics"), MakeShared<FJsonValueObject>(MetricJson));

    LastTickTime = InterimStart = FPlatformTime::Seconds();
    bIsReady = true;
}

void FTimeSeriesDataEmitterModule::UpdateMetric(FString Name, FString Description, double Value)
{
    TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
    MetricInfoJson->SetField("description", MakeShared<FJsonValueString>((TEXT("%s"), *Description)));
    MetricInfoJson->SetField("value", MakeShared<FJsonValueNumber>(Value));
    MetricJson->SetField(*Name, MakeShared<FJsonValueObject>(MetricInfoJson));
}

void FTimeSeriesDataEmitterModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

bool FTimeSeriesDataEmitterModule::IsTickableWhenPaused() const
{
    return true;
}

bool FTimeSeriesDataEmitterModule::IsTickableInEditor() const
{
    return true;
}

void FTimeSeriesDataEmitterModule::Tick(float DeltaTime)
{
    if (!FBuccaneerCommonModule::GetModule()->CVarBuccaneerEnableStats->GetBool() || !bIsReady)
    {
        // Performance profiling hasn't been inititialized. Don't continue
        return;
    }

    double NowTime = FPlatformTime::Seconds();

    InterimFrameCount++;
    double FrameTime = NowTime - LastTickTime;
    // Ignore frames that take longer than 250ms. Count these as a hang
    if (FrameTime > 0.25)
    {
        InterimHangCount++;
        FSemanticEventEmitterModule *Module = FSemanticEventEmitterModule::GetModule();
        if (Module)
        {
            Module->EmitSemanticEvent(FString(TEXT("warning")), FString(TEXT("Frame hung")));
        }
    }
    else
    {
        double GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        double GPUFrameTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles(0));
        double RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        double RHIThreadTime = FPlatformTime::ToMilliseconds(GRHIThreadTime);

        InterimMeanFrameTime = COMPUTE_MEAN(InterimMeanFrameTime, FrameTime * 1000, InterimFrameCount);
        InterimMeanGameThreadTime = COMPUTE_MEAN(InterimMeanGameThreadTime, GameThreadTime, InterimFrameCount);
        InterimMeanGPUTime = COMPUTE_MEAN(InterimMeanGPUTime, GPUFrameTime, InterimFrameCount);
        InterimMeanRenderThreadTime = COMPUTE_MEAN(InterimMeanRenderThreadTime, RenderThreadTime, InterimFrameCount);
        InterimMeanRHIThreadTime = COMPUTE_MEAN(InterimMeanRHIThreadTime, RHIThreadTime, InterimFrameCount);

        ComputeUsedMemory();
    }
    if ((NowTime - InterimStart) >= InterimDuration)
    {
        PushStatsHTTP();
        InterimStart = NowTime;
        InterimHangCount = 0;
        InterimFrameCount = 1;
    }
    LastTickTime = NowTime;
}

void FTimeSeriesDataEmitterModule::ComputeUsedMemory()
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();

    const unsigned int BitsPerMB = (8u * 1024u * 1024u);
    UsedVirtualMemory = static_cast<double>(MemoryStats.UsedVirtual) / BitsPerMB;
    UsedPhysicalMemory = static_cast<double>(MemoryStats.UsedPhysical) / BitsPerMB;

    TArray<FStatMessage> Stats;
    GetPermanentStats(Stats);

    FName NAME_STATGROUP_RHI(FStatGroup_STATGROUP_RHI::GetGroupName());
    int64 TotalMemory = 0;
    for (int32 Index = 0; Index < Stats.Num(); Index++)
    {
        FStatMessage const &Meta = Stats[Index];
        FName LastGroup = Meta.NameAndInfo.GetGroupName();
        if (LastGroup == NAME_STATGROUP_RHI && Meta.NameAndInfo.GetFlag(EStatMetaFlags::IsMemory))
        {
            TotalMemory += Meta.GetValue_int64();
        }
    }
    UsedGPUMemory = (double)(TotalMemory / 1024.f / 1024.f);
    // UE_LOG(TimeSeriesDataEmitter, VeryVerbose, TEXT("Used GPU Memory: %.3fMB"), UsedGPUMemory);
}

void FTimeSeriesDataEmitterModule::PushStatsHTTP()
{
    // Collected Metrics
    //                name       description       value
    UpdateMetric("mean_fps", "The average fps", InterimMeanFrameTime != 0.0 ? (float)(1000.0 / InterimMeanFrameTime) : 0.0f);
    UpdateMetric("mean_frametime", "The average frametime", InterimMeanFrameTime);
    UpdateMetric("mean_gamethreadtime", "The average game thread time", InterimMeanGameThreadTime);
    UpdateMetric("mean_gputime", "The average gpu time", InterimMeanGPUTime);
    UpdateMetric("mean_rendertime", "The average render thread time", InterimMeanRenderThreadTime);
    UpdateMetric("mean_rhithreadtime", "The average rhi thread time", InterimMeanRHIThreadTime);
    UpdateMetric("memory_virtual", "The virtual memory usage", UsedVirtualMemory);
    UpdateMetric("memory_physical", "The physical memory usage", UsedPhysicalMemory);
    UpdateMetric("memory_gpu", "The gpu memory usage", UsedGPUMemory);
    UpdateMetric("num_hangs", "The number of frames hung in the recording interval", InterimHangCount);

    FBuccaneerCommonModule::GetModule()->SendStats(JsonObject);
}

TStatId FTimeSeriesDataEmitterModule::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FTimeSeriesDataEmitterModule, STATGROUP_Tickables);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTimeSeriesDataEmitterModule, TimeSeriesDataEmitter)