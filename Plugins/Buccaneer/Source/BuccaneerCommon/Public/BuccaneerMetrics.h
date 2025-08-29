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
	TArray<FBuccaneerMetric> Metrics; // For global metrics
	TMap<FString, TArray<FBuccaneerMetric>> PlayerMetrics; // For per-player metrics

	/**
	 * @brief Converts the FMetricsCollection to a nested FJsonObject.
	 * @return A TSharedPtr to the created FJsonObject.
	 */
	TSharedPtr<FJsonObject> ToJsonNested() const;

	/**
	 * @brief Converts the FMetricsCollection to an unnested FJsonObject without descriptions. 
	 * @return A TSharedPtr to the created FJsonObject.
	 */
	TSharedPtr<FJsonObject> ToJsonUnnested() const;
};
