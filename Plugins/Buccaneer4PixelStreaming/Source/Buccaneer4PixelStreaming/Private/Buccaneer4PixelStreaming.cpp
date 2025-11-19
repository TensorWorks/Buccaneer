// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "Buccaneer4PixelStreaming.h"
#include "IBuccaneerStatsModule.h"
#include "Logging.h"
#include "PixelStreamingDelegates.h"
#include "Buccaneer4PixelStreamingSettings.h"

TMap<FString, FString> StatDescriptionMap = {
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
	{"captureFps", "capture fps"}};

void FBuccaneer4PixelStreamingModule::StartupModule()
{
	LoggingStart = FPlatformTime::Seconds();
	ReportingInterval = 1;

	if (UPixelStreamingDelegates *Delegates = UPixelStreamingDelegates::GetPixelStreamingDelegates())
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
	if (!UBuccaneer4PixelStreamingSettings::CVarEnabled.GetValueOnAnyThread())
	{
		return;
	}

	FBuccaneerMetric NewMetric;
	NewMetric.Name = StatName.ToString();
	if (const FString* Description = StatDescriptionMap.Find(StatName.ToString()))
	{
		NewMetric.Description = *Description;
	}
	else
	{
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Unknown stat {0}", StatName.ToString());
		NewMetric.Description = StatName.ToString(); // Default description
	}
	NewMetric.Value = StatValue;

	// Application level stats go into SingleValueMetrics
	if(PlayerId == TEXT("Application"))
	{
		bool bFound = false;
		for (FBuccaneerMetric& Metric : MetricsCollection.SingleValueMetrics)
		{
			if (Metric.Name == NewMetric.Name)
			{
				Metric.Value = NewMetric.Value;
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			MetricsCollection.SingleValueMetrics.Add(NewMetric);
		}
	}
	else
	{
		// All other metrics are player-specific
		TArray<FBuccaneerMetric>& PlayerStats = MetricsCollection.GroupedMetrics.FindOrAdd(PlayerId);
		bool bFound = false;
		for (FBuccaneerMetric& Metric : PlayerStats)
		{
			if (Metric.Name == NewMetric.Name)
			{
				Metric.Value = NewMetric.Value;
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			PlayerStats.Add(NewMetric);
		}
	}

	double NowTime = IBuccaneerStatsModule::GetStatsTimestamp();
	if ((NowTime - LoggingStart) >= ReportingInterval)
	{
		LoggingStart = NowTime;
		MetricsCollection.Timestamp = LoggingStart;

		IBuccaneerCommonModule::Get().SendMetrics(MetricsCollection);

		MetricsCollection.SingleValueMetrics.Empty();
		MetricsCollection.GroupedMetrics.Empty();
	}
}

IMPLEMENT_MODULE(FBuccaneer4PixelStreamingModule, Buccaneer4PixelStreaming)