// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "Buccaneer4PixelStreaming2.h"

#include "Logging.h"
#include "Buccaneer4PixelStreaming2Settings.h"

TMap<FString, FString> PSStatDescriptionMap = {
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

void FBuccaneer4PixelStreaming2Module::StartupModule()
{
    LoggingStart = FPlatformTime::Seconds();

    JsonObject =  MakeShareable(new FJsonObject());

	if (UPixelStreaming2Delegates* Delegates = UPixelStreaming2Delegates::Get())
	{
		Delegates->OnStatChangedNative.AddRaw(this, &FBuccaneer4PixelStreaming2Module::ConsumeStat);
	}
}

void FBuccaneer4PixelStreaming2Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FBuccaneer4PixelStreaming2Module::ConsumeStat(FString PlayerId, FName StatName, float StatValue)
{
    if (!UBuccaneer4PixelStreaming2Settings::CVarEnabled.GetValueOnAnyThread() || PlayerId == TEXT("Application") || UBuccaneer4PixelStreaming2Settings::CVarReportingInterval.GetValueOnAnyThread() <= 0)
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
	if(JsonObject->TryGetObjectField(*StatName.ToString(), MetricJson))
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
        if(!PSStatDescriptionMap.Contains(*StatName.ToString()))
        {
            UE_LOGFMT(LogBuccaneer4PixelStreaming2, Verbose, "{0}", StatName.ToString());
            return;
        }
        TSharedPtr<FJsonObject> NewMetricJson = MakeShareable(new FJsonObject());
        NewMetricJson->SetField("description", MakeShared<FJsonValueString>(*PSStatDescriptionMap[*StatName.ToString()]));

		
        TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
        ValueJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));

        TArray<TSharedPtr<FJsonValue>> ValueArray;
        ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));
		
        NewMetricJson->SetArrayField((TEXT("value")), ValueArray);

		JsonObject->SetObjectField(*StatName.ToString(), NewMetricJson);
	}

    double NowTime = FPlatformTime::Seconds();
	if ((NowTime - LoggingStart) >= UBuccaneer4PixelStreaming2Settings::CVarReportingInterval.GetValueOnAnyThread())
	{
		LoggingStart = NowTime;
        TSharedPtr<FJsonObject> PayloadJson =  MakeShareable(new FJsonObject());
        PayloadJson->SetObjectField(TEXT("metrics"), JsonObject);
		IBuccaneerCommonModule::Get().SendStats(PayloadJson);

        JsonObject = MakeShareable(new FJsonObject());
	}
}
	
IMPLEMENT_MODULE(FBuccaneer4PixelStreaming2Module, Buccaneer4PixelStreaming2)