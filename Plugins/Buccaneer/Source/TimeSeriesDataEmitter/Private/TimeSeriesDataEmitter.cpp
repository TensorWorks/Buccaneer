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
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// TODO: Parse command line
    BuccaneerCommonModule = FBuccaneerCommonModule::GetModule();
    if(!BuccaneerCommonModule->bEnableStats)
    {
        return;
    }
    // Add a callback to setup this module when Buccaneer Common has finished setting up
	BuccaneerCommonModule->SetupComplete.AddRaw(this, &FTimeSeriesDataEmitterModule::Setup);
}

void FTimeSeriesDataEmitterModule::Setup()
{
    UE_LOG(TimeSeriesDataEmitter, Log, TEXT("FTimeSeriesDataEmitterModule::Setup()"));
    JsonObject = MakeShareable(new FJsonObject());
    MetricJson = MakeShareable(new FJsonObject());


    // Collected Metrics
    //                name       description       type
    RegisterMetric("mean_fps", "The average fps", "gauge");
    RegisterMetric("mean_frametime", "The average frametime", "gauge");
    RegisterMetric("mean_gamethreadtime", "The average game thread time", "gauge");
    RegisterMetric("mean_gputime", "The average gpu time", "gauge");
    RegisterMetric("mean_rendertime", "The average render thread time", "gauge");
    RegisterMetric("mean_rhithreadtime", "The average rhi thread time", "gauge");
    RegisterMetric("num_hangs", "The number of frames hung in the recording interval", "gauge");
    RegisterMetric("memory_virtual", "The virtual memory usage", "gauge");
    RegisterMetric("memory_physical", "The physical memory usage", "gauge");
    RegisterMetric("memory_gpu", "The gpu memory usage", "gauge");

    // Send setup http request
    JsonObject->SetField("metadata", MakeShared<FJsonValueObject>(BuccaneerCommonModule->MetadataJson));
    JsonObject->SetField("metrics", MakeShared<FJsonValueObject>(MetricJson));

    BuccaneerCommonModule->SendHTTP((BuccaneerCommonModule->StatsEmitterURL + FString("/setup")), JsonObject);
    
    
    JsonObject = MakeShareable(new FJsonObject());
	LastTickTime = InterimStart = FPlatformTime::Seconds();
    bIsReady = true;
}

void FTimeSeriesDataEmitterModule::RegisterMetric(FString Name, FString Description, FString Type) 
{
    TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
    MetricInfoJson->SetField("description", MakeShared<FJsonValueString>((TEXT("%s"), *Description)));
    MetricInfoJson->SetField("type", MakeShared<FJsonValueString>((TEXT("%s"), *Type)));
    MetricJson->SetField(*Name, MakeShared<FJsonValueObject>(MetricInfoJson));
}

void FTimeSeriesDataEmitterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FTimeSeriesDataEmitterModule::IsTickableWhenPaused() const
{
	return false;
}

bool FTimeSeriesDataEmitterModule::IsTickableInEditor() const
{
	return false;
}

void FTimeSeriesDataEmitterModule::Tick(float DeltaTime) 
{
	 if(!BuccaneerCommonModule->bEnableStats || !bIsReady) 
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
        FSemanticEventEmitterModule* Module = FSemanticEventEmitterModule::GetModule();
        if(Module)
        {
            Module->EmitSemanticEvent(FString(TEXT("warning")), FString(TEXT("Frame hung")));
        }
    } 
    else 
    {
        double GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
        // UE_LOG(TimeSeriesDataEmitter, Warning, TEXT("%d"), GRHIDeviceId);
        double GPUFrameTime = 0;
        if(GRHIDeviceId == 0)
        {
            GPUFrameTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles(GRHIDeviceId));
        }
	    double RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	    double RHIThreadTime = FPlatformTime::ToMilliseconds(GRHIThreadTime);

        InterimMeanFrameTime = COMPUTE_MEAN(InterimMeanFrameTime, FrameTime * 1000, InterimFrameCount);
        InterimMeanGameThreadTime = COMPUTE_MEAN(InterimMeanGameThreadTime, GameThreadTime, InterimFrameCount);
        InterimMeanGPUTime = COMPUTE_MEAN(InterimMeanGPUTime, GPUFrameTime, InterimFrameCount);
        InterimMeanRenderThreadTime = COMPUTE_MEAN(InterimMeanRenderThreadTime, RenderThreadTime, InterimFrameCount);
        InterimMeanRHIThreadTime = COMPUTE_MEAN(InterimMeanRHIThreadTime, RHIThreadTime, InterimFrameCount);
        
        ComputeUsedMemory();
    }
    if ( (NowTime - InterimStart) >= InterimDuration )
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
	UsedGPUMemory = 0;
	for (int32 Index = 0; Index < Stats.Num(); Index++)
	{
		FStatMessage const& Meta = Stats[Index];
		FName LastGroup = Meta.NameAndInfo.GetGroupName();
		if (LastGroup == NAME_STATGROUP_RHI && Meta.NameAndInfo.GetFlag(EStatMetaFlags::IsMemory))
		{
			UsedGPUMemory += Meta.GetValue_double();
		}
	}
}

void FTimeSeriesDataEmitterModule::PushStatsHTTP() 
{
    JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *BuccaneerCommonModule->ID)));
    JsonObject->SetField("mean_fps", MakeShared<FJsonValueNumber>(InterimMeanFrameTime != 0.0 ? (float)(1000.0 / InterimMeanFrameTime) : 0.0f));
    JsonObject->SetField("mean_frametime", MakeShared<FJsonValueNumber>(InterimMeanFrameTime));
	JsonObject->SetField("mean_gamethreadtime", MakeShared<FJsonValueNumber>(InterimMeanGameThreadTime));
	JsonObject->SetField("mean_gputime", MakeShared<FJsonValueNumber>(InterimMeanGPUTime));
    JsonObject->SetField("mean_rendertime", MakeShared<FJsonValueNumber>(InterimMeanRenderThreadTime));
    JsonObject->SetField("mean_rhithreadtime", MakeShared<FJsonValueNumber>(InterimMeanRHIThreadTime));
	JsonObject->SetField("memory_virtual", MakeShared<FJsonValueNumber>(UsedVirtualMemory));
	JsonObject->SetField("memory_physical", MakeShared<FJsonValueNumber>(UsedPhysicalMemory));
    JsonObject->SetField("memory_gpu", MakeShared<FJsonValueNumber>(UsedGPUMemory));
    JsonObject->SetField("num_hangs", MakeShared<FJsonValueNumber>(InterimHangCount));
    BuccaneerCommonModule->SendHTTP((BuccaneerCommonModule->StatsEmitterURL + FString("/stats")), JsonObject);
}

TStatId FTimeSeriesDataEmitterModule::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FTimeSeriesDataEmitterModule, STATGROUP_Tickables);
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTimeSeriesDataEmitterModule, TimeSeriesDataEmitter)