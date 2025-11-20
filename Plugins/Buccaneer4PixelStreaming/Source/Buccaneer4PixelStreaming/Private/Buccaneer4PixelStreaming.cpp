// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "Buccaneer4PixelStreaming.h"
#include "IBuccaneerStatsModule.h"
#include "Logging.h"
#include "PixelStreamingDelegates.h"
#include "Buccaneer4PixelStreamingSettings.h"

TMap<FString, FString> StatDescriptionMap = {
	{"jitterBufferDelay", "Current playout delay introduced by the jitter buffer"},
	{"framesSent", "Total number of video frames sent"},
	{"framesPerSecond", "Current streaming rate in frames (fps)"},
	{"framesReceived", "Total number of video frames received"},
	{"framesDropped", "Number of frames dropped to preserve bitrate"},
	{"framesDecoded", "Total number of video frames decoded"},
	{"framesCorrupted", "Total number of corrupted video frames"},
	{"partialFramesLost", "Number of partially lost frames"},
	{"fullFramesLost", "Number of fully lost frames"},
	{"hugeFramesSent", "Number of huge frames sent"},
	{"jitterBufferTargetDelay", "Target delay the jitter buffer aims to maintain"},
	{"interruptionCount", "Number of playback interruptions"},
	{"totalInterruptionDuration", "Total duration of playback interruptions"},
	{"freezeCount", "Number of playback freezes (lags of 300ms+)"},
	{"pauseCount", "Number of playback pauses"},
	{"totalFreezesDuration", "Total duration of playback freezes"},
	{"totalPausesDuration", "Total duration of playback pauses"},
	{"firCount", "Number of Full Intra Request (FIR) packets sent"},
	{"pliCount", "Number of Picture Loss Indication (PLI) packets sent"},
	{"nackCount", "Number of Negative Acknowledgement (NACK) packets sent"},
	{"sliCount", "Number of Slice Loss Indication (SLI) packets sent"},
	{"retransmittedBytesSent", "Number of retransmitted bytes sent"},
	{"totalEncodedBytesTarget", "Target total encoded bytes over time"},
	{"keyFramesEncoded", "Total number of key frames encoded"},
	{"frameWidth", "Width of the video frame"},
	{"frameHeight", "Height of the video frame"},
	{"bytesSent", "Total number of bytes sent"},
	{"qpSum", "Sum of quantization parameters for encoded frames, a good measure video quality"},
	{"totalEncodeTime", "Total encoding time (ms)"},
	{"totalPacketSendDelay", "Total packet send delay (ms)"},
	{"packetSendDelay", "Packet send delay (ms)"},
	{"framesEncoded", "Total number of frames encoded"},
	{"transmitFps", "Transmit frames per second (fps)"},
	{"bitrate", "Bitrate (kb/s)"},
	{"qp", "Quantization parameter used for video encoding, a good measure of video quality"},
	{"encodeTime", "Encode time (ms)"},
	{"encodeFps", "Encode frames per second (fps)"},
	{"captureToSend", "Capture to send time (ms)"},
	{"captureFps", "Capture frames per second (fps)"}};
	
void FBuccaneer4PixelStreamingModule::StartupModule()
{
	LoggingStart = FPlatformTime::Seconds();

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
	const double ReportingIntervalSeconds = UBuccaneer4PixelStreamingSettings::CVarReportingInterval.GetValueOnAnyThread();
	if (ReportingIntervalSeconds > 0.0 && (NowTime - LoggingStart) >= ReportingIntervalSeconds)
	{
		LoggingStart = NowTime;
		MetricsCollection.Timestamp = LoggingStart;

		IBuccaneerCommonModule::Get().SendMetrics(MetricsCollection);

		MetricsCollection.SingleValueMetrics.Empty();
		MetricsCollection.GroupedMetrics.Empty();
	}
}

IMPLEMENT_MODULE(FBuccaneer4PixelStreamingModule, Buccaneer4PixelStreaming)