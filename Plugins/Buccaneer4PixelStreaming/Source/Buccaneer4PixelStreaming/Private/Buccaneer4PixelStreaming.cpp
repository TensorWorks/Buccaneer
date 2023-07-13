// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buccaneer4PixelStreaming.h"
#include "Logging/LogMacros.h"
#include "PixelStreamingDelegates.h"

#define LOCTEXT_NAMESPACE "FBuccaneer4PixelStreamingModule"


DEFINE_LOG_CATEGORY(BuccaneerPixelStreaming);

namespace Buccaneer4PixelStreaming
{
	
}

void FBuccaneer4PixelStreamingModule::StartupModule()
{
    StatDescriptionMap = {
        {"jitterBufferDelay", "jitterBufferDelay"},
        {"framesSent", "framesSent"},
	    {"framesPerSecond", "framesPerSecond"},
	    {"framesReceived", "framesReceived"},
	    {"framesDropped", "framesDropped"},
	    {"framesDecoded", "framesDecoded"},
	    {"framesCorrupted", "framesCorrupted"},
	    {"partialFramesLost", "partialFramesLost"},
	    {"fullFramesLost", "fullFramesLost"},
	    {"hugeFramesSent", "hugeFramesSent"},
	    {"jitterBufferTargetDelay", "jitterBufferTargetDelay"},
	    {"interruptionCount", "interruptionCount"},
	    {"totalInterruptionDuration", "totalInterruptionDuration"},
	    {"freezeCount", "freezeCount"},
	    {"pauseCount", "pauseCount"},
	    {"totalFreezesDuration", "totalFreezesDuration"},
	    {"totalPausesDuration", "totalPausesDuration"},
	    {"firCount", "firCount"},
	    {"pliCount", "pliCount"},
	    {"nackCount", "nackCount"},
	    {"sliCount", "sliCount"},
	    {"retransmittedBytesSent", "retransmittedBytesSent"},
	    {"totalEncodedBytesTarget", "totalEncodedBytesTarget"},
	    {"keyFramesEncoded", "keyFramesEncoded"},
	    {"frameWidth", "frameWidth"},
	    {"frameHeight", "frameHeight"},
	    {"bytesSent", "bytesSent"},
	    {"qpSum", "qpSum"},
	    {"totalEncodeTime", "totalEncodeTime"},
	    {"totalPacketSendDelay", "totalPacketSendDelay"},
        {"packetSendDelay", "packetSendDelay"},
	    {"framesEncoded", "framesEncoded"},
	    {"transmitFps", "transmit fps"},
	    {"bitrate", "bitrate (kb/s)"},
	    {"qp", "qp"},
	    {"encodeTime", "encode time (ms)"},
	    {"encodeFps", "encode fps"},
	    {"captureToSend", "capture to send (ms)"},
	    {"captureFps", "capture fps"}
    };

	CVarBuccaneer4PixelStreamingEnableStats = IConsoleManager::Get().RegisterConsoleVariable(
		TEXT("Buccaneer4PixelStreaming.EnableStats"),
		true,
		TEXT("Disables the collection and logging of Pixel Streaming stats with Buccaneer"),
		ECVF_Default);

    Setup();
}

void FBuccaneer4PixelStreamingModule::Setup()
{
    FBuccaneerCommonModule::ParseCommandLineOption(TEXT("Buccaneer4PixelStreamingEnableStats"), CVarBuccaneer4PixelStreamingEnableStats);

	LoggingStart = FPlatformTime::Seconds();
	ReportingInterval = 1;

    JsonObject =  MakeShareable(new FJsonObject());

	if (UPixelStreamingDelegates* Delegates = UPixelStreamingDelegates::GetPixelStreamingDelegates())
	{
		Delegates->OnStatChangedNative.AddRaw(this, &FBuccaneer4PixelStreamingModule::ConsumeStat);
	}
}

void FBuccaneer4PixelStreamingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FBuccaneer4PixelStreamingModule::ConsumeStat(FPixelStreamingPlayerId PlayerId, FName StatName, float StatValue)
{
    if(!CVarBuccaneer4PixelStreamingEnableStats->GetBool() || PlayerId == TEXT("Application"))
	{
		return;
	}
    /**
    * "{StatName}": {
    *      "description": "{StatDescription}",
    *      "value": [
    *          "{PlayerId}": {StatValue}
    *      ]
    * }
    */

    const TSharedPtr<FJsonObject>* MetricJson = nullptr;
	if(JsonObject->TryGetObjectField((TEXT("%s"), *StatName.ToString()), MetricJson))
	{
		TArray<TSharedPtr<FJsonValue>> ValueArray = (*MetricJson)->GetArrayField(TEXT("value"));

        bool bRequiresCreation = true;
        for (int i = 0; i < ValueArray.Num(); i++) 
        {
            const TSharedPtr<FJsonObject> ValueJson = ValueArray[i]->AsObject();
			double val;
			if(ValueJson->TryGetNumberField(*PlayerId, val))
			{
				// This metric already has this player id, update the value accordingly
				ValueJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));
				bRequiresCreation = false;
				break;
			}
        }
	
		if(bRequiresCreation)
		{
			TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
            ValueJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));

            ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));

            (*MetricJson)->SetArrayField((TEXT("value")), ValueArray);
		}
	}
	else
	{
        if(!StatDescriptionMap.Contains(*StatName.ToString()))
        {
            UE_LOG(BuccaneerPixelStreaming, Log, TEXT("%s"), *StatName.ToString());
            return;
        }
        TSharedPtr<FJsonObject> NewMetricJson = MakeShareable(new FJsonObject());
        NewMetricJson->SetField("description", MakeShared<FJsonValueString>((TEXT("%s"), *StatDescriptionMap[*StatName.ToString()])));

		
        TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
        ValueJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));

        TArray<TSharedPtr<FJsonValue>> ValueArray;
        ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));
		
        NewMetricJson->SetArrayField((TEXT("value")), ValueArray);

		JsonObject->SetObjectField((TEXT("%s"), *StatName.ToString()), NewMetricJson);
	}

    double NowTime = FPlatformTime::Seconds();
	if ( (NowTime - LoggingStart) >= ReportingInterval )
	{
		LoggingStart = NowTime;
        TSharedPtr<FJsonObject> PayloadJson =  MakeShareable(new FJsonObject());
        PayloadJson->SetObjectField(TEXT("metrics"), JsonObject);
		FBuccaneerCommonModule::GetModule()->SendStats(PayloadJson);

        JsonObject =  MakeShareable(new FJsonObject());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBuccaneer4PixelStreamingModule, Buccaneer4PixelStreaming)