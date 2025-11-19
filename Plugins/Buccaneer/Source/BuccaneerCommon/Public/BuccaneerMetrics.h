/**
 * @file BuccaneerStats.h
 * @brief Defines the data structures for collecting and storing statistics.
 */

#pragma once

#include "Dom/JsonObject.h"

/**
 * @struct FBuccaneerMetric
 * @brief Represents a single metric, including its name, description, and value.
 */
struct FBuccaneerMetric
{
	FString Name;
	FString Description;
	double Value;
};

/**
 * @struct FMetricsCollection
 * @brief Represents a collection of metrics captured at a specific moment in time.
 */
struct FMetricsCollection
{
	double Timestamp;

	// These metrics have a single value each.
	// They can be used to record things such as global metrics, application-wide stats, session stats, etc.
	TArray<FBuccaneerMetric> SingleValueMetrics;

	// These metrics are stored in logical groups by unique ids (key: player id, value: array of metrics).
	// For example, they could be used for per-peer metrics for Pixel Streaming or per-player metrics in a local multiplayer game.
	TMap<FString, TArray<FBuccaneerMetric>> GroupedMetrics;

	/**
	 * @brief Converts the FMetricsCollection to a nested FJsonObject.
	 * @return A TSharedPtr to the created FJsonObject.
	 */
	TSharedPtr<FJsonObject> ToJson() const;
};
