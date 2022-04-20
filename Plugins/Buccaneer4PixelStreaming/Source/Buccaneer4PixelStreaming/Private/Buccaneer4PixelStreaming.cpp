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

	CVarBuccaneer4PixelStreamingEnableStats = IConsoleManager::Get().RegisterConsoleVariable(
		TEXT("Buccaneer4PixelStreaming.EnableStats"),
		true,
		TEXT("Disables the collection and logging of Pixel Streaming stats with Buccaneer"),
		ECVF_Default);

	BuccaneerCommonModule = FBuccaneerCommonModule::GetModule();
	BuccaneerCommonModule->ParseCommandLineOption(TEXT("Buccaneer4PixelStreamingEnableStats"), CVarBuccaneer4PixelStreamingEnableStats);
	if(!CVarBuccaneer4PixelStreamingEnableStats->GetBool())
	{
		return;
	}
	
	if(BuccaneerCommonModule->StatsEmitterURL == "")
	{
		// No URL. No point in continuing
		UE_LOG(BuccaneerPixelStreaming, Log, TEXT("Buccaneer Pixel Streaming logging is disabled, provide `StatsEmitterURL` cmd-args to enable it"));
		return;
	}

	// Add a callback to setup this module when Buccaneer Common has finished setting up
	BuccaneerCommonModule->SetupComplete.AddRaw(this, &FBuccaneer4PixelStreamingModule::Setup);
}

void FBuccaneer4PixelStreamingModule::HandlePlayerDisconnect(FPixelStreamingPlayerId PlayerId, bool WasQualityController)
{
	UE_LOG(BuccaneerPixelStreaming, Log, TEXT("Player disconnected: %s"), *PlayerId);
	TSharedPtr<FJsonObject> TempJson = MakeShareable(new FJsonObject());
	TempJson->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *BuccaneerCommonModule->ID)));
	TempJson->SetField("playerId", MakeShared<FJsonValueString>((TEXT("%s"), *PlayerId)));
	BuccaneerCommonModule->SendHTTP((BuccaneerCommonModule->StatsEmitterURL + FString("/deletePlayer")), TempJson);

	JsonObject = MakeShareable(new FJsonObject());
}

void FBuccaneer4PixelStreamingModule::Setup()
{
	UE_LOG(BuccaneerPixelStreaming, Log, TEXT("FBuccaneer4PixelStreamingModule::Setup()"));
	JsonObject = MakeShareable(new FJsonObject());
	SetupJson = MakeShareable(new FJsonObject());
    MetricJson = MakeShareable(new FJsonObject());

	RegisterMetric("jitterBufferDelay", "jitterBufferDelay", "gauge");
	RegisterMetric("framesSent", "framesSent", "gauge");
	RegisterMetric("framesPerSecond", "framesPerSecond", "gauge");
	RegisterMetric("framesReceived", "framesReceived", "gauge");
	RegisterMetric("framesDropped", "framesDropped", "gauge");
	RegisterMetric("framesDecoded", "framesDecoded", "gauge");
	RegisterMetric("framesCorrupted", "framesCorrupted", "gauge");
	RegisterMetric("partialFramesLost", "partialFramesLost", "gauge");
	RegisterMetric("fullFramesLost", "fullFramesLost", "gauge");
	RegisterMetric("hugeFramesSent", "hugeFramesSent", "gauge");
	RegisterMetric("jitterBufferTargetDelay", "jitterBufferTargetDelay", "gauge");
	RegisterMetric("interruptionCount", "interruptionCount", "gauge");
	RegisterMetric("totalInterruptionDuration", "totalInterruptionDuration", "gauge");
	RegisterMetric("freezeCount", "freezeCount", "gauge");
	RegisterMetric("pauseCount", "pauseCount", "gauge");
	RegisterMetric("totalFreezesDuration", "totalFreezesDuration", "gauge");
	RegisterMetric("totalPausesDuration", "totalPausesDuration", "gauge");
	RegisterMetric("firCount", "firCount", "gauge");
	RegisterMetric("pliCount", "pliCount", "gauge");
	RegisterMetric("nackCount", "nackCount", "gauge");
	RegisterMetric("sliCount", "sliCount", "gauge");
	RegisterMetric("retransmittedBytesSent", "retransmittedBytesSent", "gauge");
	RegisterMetric("totalEncodedBytesTarget", "totalEncodedBytesTarget", "gauge");
	RegisterMetric("keyFramesEncoded", "keyFramesEncoded", "gauge");
	RegisterMetric("frameWidth", "frameWidth", "gauge");
	RegisterMetric("frameHeight", "frameHeight", "gauge");
	RegisterMetric("bytesSent", "bytesSent", "gauge");
	RegisterMetric("qpSum", "qpSum", "gauge");
	RegisterMetric("totalEncodeTime", "totalEncodeTime", "gauge");
	RegisterMetric("totalPacketSendDelay", "totalPacketSendDelay", "gauge");
	RegisterMetric("framesEncoded", "framesEncoded", "gauge");
	RegisterMetric("transmitFps", "transmit fps", "gauge");
	RegisterMetric("bitrate", "bitrate (kb/s)", "gauge");
	RegisterMetric("qp", "qp", "gauge");
	RegisterMetric("encodeTime", "encode time (ms)", "gauge");
	RegisterMetric("encodeFps", "encode fps", "gauge");
	RegisterMetric("captureToSend", "capture to send (ms)", "gauge");
	RegisterMetric("captureFps", "capture fps", "gauge");


	SetupJson->SetField("metadata", MakeShared<FJsonValueObject>(BuccaneerCommonModule->MetadataJson));
    SetupJson->SetField("metrics", MakeShared<FJsonValueObject>(MetricJson));
	BuccaneerCommonModule->SendHTTP((BuccaneerCommonModule->StatsEmitterURL + FString("/setup")), SetupJson);

	LoggingStart = FPlatformTime::Seconds();
	ReportingInterval = 1;

	if (UPixelStreamingDelegates* Delegates = UPixelStreamingDelegates::GetPixelStreamingDelegates())
	{
		Delegates->OnClosedConnectionNative.AddRaw(this, &FBuccaneer4PixelStreamingModule::HandlePlayerDisconnect);
		Delegates->OnStatChangedNative.AddRaw(this, &FBuccaneer4PixelStreamingModule::ConsumeStat);
	}
}

void FBuccaneer4PixelStreamingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FBuccaneer4PixelStreamingModule::RegisterMetric(FString Name, FString Description, FString Type) 
{
    TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
    MetricInfoJson->SetField("description", MakeShared<FJsonValueString>((TEXT("%s"), *Description)));
    MetricInfoJson->SetField("type", MakeShared<FJsonValueString>((TEXT("%s"), *Type)));
	MetricInfoJson->SetField("perPlayer", MakeShared<FJsonValueBoolean>(true));
    MetricJson->SetField(*Name, MakeShared<FJsonValueObject>(MetricInfoJson));
}

void FBuccaneer4PixelStreamingModule::ConsumeStat(FPixelStreamingPlayerId PlayerId, FName StatName, float StatValue)
{
	double NowTime = FPlatformTime::Seconds();

	bool bHasField = JsonObject->HasField((TEXT("%s"), *StatName.ToString()));
	if(bHasField)
	{
		TArray<TSharedPtr<FJsonValue>> ValueArray = JsonObject->GetArrayField((TEXT("%s"), *StatName.ToString()));
		bool bRequiresCreation = true;
		for (int i = 0; i < ValueArray.Num(); i++) 
		{
			const TSharedPtr<FJsonObject> temp = ValueArray[i]->AsObject();
			double val;
			bool bSuccess = temp->TryGetNumberField(*PlayerId, val);
			if(bSuccess)
			{
				// This metric already has this player id, update the value accordingly
				temp->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));
				bRequiresCreation = false;
				break;
			}
       	}
		if(bRequiresCreation)
		{
			// Metric doesn't have this player id. Append it to the ValueArray
			TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
			MetricInfoJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));

			TSharedRef< FJsonValueObject > JsonValue = MakeShareable( new FJsonValueObject( MetricInfoJson) );
			ValueArray.Add(JsonValue);
			JsonObject->SetArrayField((TEXT("%s"), *StatName.ToString()), ValueArray);
		}
	}
	else
	{
		TArray<TSharedPtr<FJsonValue>> ValueArray;
		TSharedPtr<FJsonObject> MetricInfoJson = MakeShareable(new FJsonObject());
		MetricInfoJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));

		TSharedRef< FJsonValueObject > JsonValue = MakeShareable( new FJsonValueObject( MetricInfoJson) );
		ValueArray.Add(JsonValue);
		JsonObject->SetArrayField((TEXT("%s"), *StatName.ToString()), ValueArray);
	}

	if ( (NowTime - LoggingStart) >= ReportingInterval )
	{
		LoggingStart = NowTime;
		if(BuccaneerCommonModule->ID == "")
		{
			return;
		}
   		JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *BuccaneerCommonModule->ID)));

		BuccaneerCommonModule->SendHTTP((BuccaneerCommonModule->StatsEmitterURL + FString("/stats")), JsonObject);
 		JsonObject = MakeShareable(new FJsonObject());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBuccaneer4PixelStreamingModule, Buccaneer4PixelStreaming)