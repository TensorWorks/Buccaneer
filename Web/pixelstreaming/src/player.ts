// Copyright Epic Games, Inc. All Rights Reserved.

export * from '@epicgames-ps/lib-pixelstreamingfrontend-ue5.7';
export * from '@epicgames-ps/lib-pixelstreamingfrontend-ui-ue5.7';
import { Config, PixelStreaming, Logger, LogLevel, LatencyCalculatedEvent, LatencyInfo, AggregatedStats } from '@epicgames-ps/lib-pixelstreamingfrontend-ue5.7';
import { Application, PixelStreamingApplicationStyle } from '@epicgames-ps/lib-pixelstreamingfrontend-ui-ue5.7';
import { MetricsCollection } from '@tensorworks/buccaneer-metrics-client';

const PixelStreamingApplicationStyles = new PixelStreamingApplicationStyle();
PixelStreamingApplicationStyles.applyStyleSheet();

// Expose the pixel streaming object for hooking into, tests, etc.
declare global {
    interface Window { 
        pixelStreaming: PixelStreaming;
        buccaneeredMetrics?: MetricsCollection;
    }
}

// Buccaneer server URL - can be overridden via URL param or environment
const BUCCANEER_URL = new URLSearchParams(window.location.search).get('buccaneerUrl') || 
                      import.meta.env.VITE_BUCCANEER_URL || 
                      'http://localhost:8000';

document.body.onload = function() {
    Logger.InitLogging(LogLevel.Warning, true);

    // Create a config object
    const config = new Config({ useUrlParams: true });

    // Create the main Pixel Streaming object for interfacing with the web-API of Pixel Streaming
    const stream = new PixelStreaming(config);

    const application = new Application({
        stream,
        onColorModeChanged: (isLightMode: boolean) => PixelStreamingApplicationStyles.setColorMode(isLightMode)
    });
    document.body.appendChild(application.rootElement);

    window.pixelStreaming = stream;

    // Initialize Buccaneer metrics collection
    initializeBuccaneerMetrics(stream);
}

/**
 * Initialize Buccaneer metrics collection for Pixel Streaming stats
 */
function initializeBuccaneerMetrics(stream: PixelStreaming) {
    // Get player ID from WebRTC controller
    const getPlayerId = (): string => {
        try {
            // Access the internal WebRTC controller to get player ID
            const playerId = (stream as any)._webRtcController?.playerId;
            return playerId || 'unknown';
        } catch (error) {
            console.warn('Failed to get player ID:', error);
            return 'unknown';
        }
    };

    // Create metrics collection instance
    const metrics = new MetricsCollection('pixelstreaming-frontend', {
        userAgent: navigator.userAgent,
        platform: navigator.platform,
    });

    // Store globally for external access
    window.buccaneeredMetrics = metrics;

    // Helper function to send metrics
    const sendMetrics = async (metricType: string) => {
        try {
            await metrics.send(BUCCANEER_URL);
            console.log(`[Buccaneer] ${metricType} metrics sent successfully to ${BUCCANEER_URL}`);
            metrics.clear();
        } catch (error) {
            console.error(`[Buccaneer] Failed to send ${metricType} metrics:`, error);
        }
    };

    // Helper function to process stats and store to metrics
    const processStats = (playerId: string, stats: any, prefix: string, description: string) => {
        for (const [statName, statValue] of Object.entries(stats)) {
            if (typeof statValue === 'number' && statName !== 'id' && statName !== 'timestamp') {
                metrics.storeStatByGroup(
                    playerId,
                    `${prefix}_${statName}`,
                    statValue,
                    `${description}: ${statName}`
                );
            }
        }
    };

    // Handler for stats received event
    const handleStatsReceived = async (aggregatedStatsPayload: any) => {
        const playerId = getPlayerId();
        const aggregatedStats: AggregatedStats = aggregatedStatsPayload;

		// Process all other stats using the helper function
		processStats(playerId, aggregatedStats.datachannelStats, 'datachannel', 'Pixel Streaming data channel stats');
		processStats(playerId, aggregatedStats.inboundAudioStats, 'inboundAudio', 'Pixel Streaming inbound audio stats');
		processStats(playerId, aggregatedStats.inboundVideoStats, 'inboundVideo', 'Pixel Streaming inbound video stats');
		processStats(playerId, aggregatedStats.outboundAudioStats, 'outboundAudio', 'Pixel Streaming outbound audio stats');
		processStats(playerId, aggregatedStats.outboundVideoStats, 'outboundVideo', 'Pixel Streaming outbound video stats');
		processStats(playerId, aggregatedStats.remoteOutboundAudioStats, 'remoteOutboundAudio', 'Pixel Streaming remote outbound audio stats');
		processStats(playerId, aggregatedStats.remoteOutboundVideoStats, 'remoteOutboundVideo', 'Pixel Streaming remote outbound video stats');

		if(aggregatedStats.sessionStats.videoEncoderAvgQP) {
			metrics.storeStatByGroup(
				playerId,
				"videoEncoderAvgQP",
				aggregatedStats.sessionStats.videoEncoderAvgQP,
				`Pixel Streaming session stats: videoEncoderAvgQP`
			);
		}

        await sendMetrics('Stats');
    };

    // Handler for latency calculated event
    const handleLatencyCalculated = async (latencyInfoPayload: any) => {
        const playerId = getPlayerId();
		const latencyInfo: LatencyInfo = latencyInfoPayload;

        // Push latency metrics as grouped metrics
		processStats(playerId, latencyInfo, /*no prefix*/'', 'Pixel Streaming latency stats');

        await sendMetrics('Latency');
    };

    // Register event listeners
    stream.addEventListener('statsReceived', async ({ data: { aggregatedStatsPayload } }: any) => {
        await handleStatsReceived(aggregatedStatsPayload);
    });

    stream.addEventListener('latencyCalculated', async ({ data: { latencyInfoPayload } }: any) => {
        await handleLatencyCalculated(latencyInfoPayload);
    });

    console.log(`[Buccaneer] Metrics collection initialized. Sending to ${BUCCANEER_URL}`);
}
