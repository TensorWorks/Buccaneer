// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "BuccaneerStatsModule.h"

#include "BuccaneerSettings.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "IBuccaneerEventsModule.h"
#include "Logging.h"
#include "RHI.h"
#include "Stats/Stats.h"
#include "Stats/StatsData.h"

#define COMPUTE_MEAN(CurrentMean, NewTime, FrameCount) \
    ((FrameCount - 1) * CurrentMean + NewTime) / FrameCount;

TMap<FString, FString> StatDescriptionMap = {
    { "mean_fps", "The average fps" },
    { "mean_frametime", "The average frametime" },
    { "mean_gamethreadtime", "The average game thread time" },
    { "mean_gputime", "The average gpu time" },
    { "mean_rendertime", "The average render thread time" },
    { "mean_rhithreadtime", "The average rhi thread time" },
    { "memory_virtual", "The virtual memory usage" },
    { "memory_physical", "The physical memory usage" },
    { "memory_gpu", "The gpu memory usage" },
    { "num_hangs", "The number of frames hung in the recording interval" }
};

void FBuccaneerStatsModule::StartupModule()
{
    MetricJson = MakeShareable(new FJsonObject());
    JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetField(TEXT("metrics"), MakeShared<FJsonValueObject>(MetricJson));

    LastTickTime = InterimStart = FPlatformTime::Seconds();
}

void FBuccaneerStatsModule::UpdateMetric(FString Name, double Value)
{
    if(!StatDescriptionMap.Contains(Name))
    {
        UE_LOGFMT(LogBuccaneerStats, Log, "No description for metric {0}", Name);
        return;
    }

    TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
    MetricInfoJson->SetField("description", MakeShared<FJsonValueString>((TEXT("%s"), *StatDescriptionMap[Name])));
    MetricInfoJson->SetField("value", MakeShared<FJsonValueNumber>(Value));
    MetricJson->SetField(*Name, MakeShared<FJsonValueObject>(MetricInfoJson));
}

void FBuccaneerStatsModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

bool FBuccaneerStatsModule::IsTickableWhenPaused() const
{
    return true;
}

bool FBuccaneerStatsModule::IsTickableInEditor() const
{
    return true;
}

void FBuccaneerStatsModule::Tick(float DeltaTime)
{
    if (!UBuccaneerSettings::CVarEnableStats->GetBool())
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
        IBuccaneerEventsModule::Get().EmitEvent(TEXT("warning"), TEXT("Frame hung"));
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

void FBuccaneerStatsModule::ComputeUsedMemory()
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();

    const unsigned int BytesPerMB = (8u * 1024u * 1024u);
    UsedVirtualMemory = static_cast<double>(MemoryStats.UsedVirtual) / BytesPerMB;
    UsedPhysicalMemory = static_cast<double>(MemoryStats.UsedPhysical) / BytesPerMB;

#if !UE_BUILD_SHIPPING
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
#endif    
}

void FBuccaneerStatsModule::PushStatsHTTP()
{
    // Collected Metrics
    //              name           value
    UpdateMetric("mean_fps", InterimMeanFrameTime != 0.0 ? (float)(1000.0 / InterimMeanFrameTime) : 0.0f);
    UpdateMetric("mean_frametime", InterimMeanFrameTime);
    UpdateMetric("mean_gamethreadtime", InterimMeanGameThreadTime);
    UpdateMetric("mean_gputime", InterimMeanGPUTime);
    UpdateMetric("mean_rendertime", InterimMeanRenderThreadTime);
    UpdateMetric("mean_rhithreadtime", InterimMeanRHIThreadTime);
    UpdateMetric("memory_virtual", UsedVirtualMemory);
    UpdateMetric("memory_physical", UsedPhysicalMemory);
    UpdateMetric("memory_gpu", UsedGPUMemory);
    UpdateMetric("num_hangs", InterimHangCount);

    IBuccaneerCommonModule::Get().SendStats(JsonObject);
}

TStatId FBuccaneerStatsModule::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FBuccaneerStatsModule, STATGROUP_Tickables);
}

#undef COMPUTE_MEAN

IMPLEMENT_MODULE(FBuccaneerStatsModule, BuccaneerStats)