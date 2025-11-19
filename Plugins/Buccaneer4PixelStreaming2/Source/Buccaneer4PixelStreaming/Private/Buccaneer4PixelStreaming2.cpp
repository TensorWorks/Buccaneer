// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "Buccaneer4PixelStreaming2.h"
#include "IBuccaneerStatsModule.h"
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
	{"captureFps", "capture fps"}};

void FBuccaneer4PixelStreaming2Module::StartupModule()
{
	LoggingStart = FPlatformTime::Seconds();

	if (UPixelStreaming2Delegates *Delegates = UPixelStreaming2Delegates::Get())
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
	if (!UBuccaneer4PixelStreaming2Settings::CVarEnabled.GetValueOnAnyThread() || UBuccaneer4PixelStreaming2Settings::CVarReportingInterval.GetValueOnAnyThread() <= 0)
	{
		return;
	}

	FBuccaneerMetric NewMetric;
	NewMetric.Name = StatName.ToString();
	if (const FString* Description = PSStatDescriptionMap.Find(StatName.ToString()))
	{
		NewMetric.Description = *Description;
	}
	else
	{
		UE_LOGFMT(LogBuccaneer4PixelStreaming2, Verbose, "Unknown stat {0}", StatName.ToString());
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
	if ((NowTime - LoggingStart) >= UBuccaneer4PixelStreaming2Settings::CVarReportingInterval.GetValueOnAnyThread())
	{
		LoggingStart = NowTime;
		MetricsCollection.Timestamp = LoggingStart;

		IBuccaneerCommonModule::Get().SendMetrics(MetricsCollection);

		MetricsCollection.SingleValueMetrics.Empty();
		MetricsCollection.GroupedMetrics.Empty();
	}
}

IMPLEMENT_MODULE(FBuccaneer4PixelStreaming2Module, Buccaneer4PixelStreaming2)