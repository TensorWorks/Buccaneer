#include "BuccaneerMetrics.h"

namespace
{
	// Helper function to format metric names to be Prometheus-compatible
	// Valid Prometheus names must be alphanumeric or underscores
	FString FormatMetricName(const FString& Name)
	{
		FString FormattedName = Name;
		FormattedName.ReplaceInline(TEXT("-"), TEXT("_"));
		FormattedName.ReplaceInline(TEXT("."), TEXT("_"));
		FormattedName.ReplaceInline(TEXT(" "), TEXT("_"));
		FormattedName.ReplaceInline(TEXT("/"), TEXT("_"));
		FormattedName.ReplaceInline(TEXT("\\"), TEXT("_"));
		FormattedName.ReplaceInline(TEXT("("), TEXT(""));
		FormattedName.ReplaceInline(TEXT(")"), TEXT(""));
		FormattedName.ReplaceInline(TEXT("\""), TEXT(""));
		FormattedName.ReplaceInline(TEXT("'"), TEXT(""));
		FormattedName.ReplaceInline(TEXT(">"), TEXT(""));
		FormattedName.ReplaceInline(TEXT("<"), TEXT(""));
		return FormattedName;
	}
}

TSharedPtr<FJsonObject> FMetricsCollection::ToJson() const
{

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> MetricsJson = MakeShareable(new FJsonObject());

	/**
	* This is the format that the Buccaneer server wants from "single value" metrics.
	*
	* Single value metrics could be:
	* - Global application stats (like framerate, memory usage, etc)
	* - Gameplay stats that are not per-player (like play time, session unix timestamp, session id etc)
	* - Any other type of stats where there is only one value for the stat
	*
    * "{MetricName}": {
    *      "description": "{StatDescription}",
    *      "value": {StatValue}
    * }
	* Example:
	* "memory_used": {
    *      "description": "The memory used by the game",
    *      "value": 1024
    * }
    */

	// Build JSON object using the "single value metrics" we have stored in the FMetricsCollection
	for (const FBuccaneerMetric &Stat : SingleValueMetrics)
	{
		FString MetricName = FormatMetricName(Stat.Name);

		TSharedPtr<FJsonObject> MetricJson = MakeShareable(new FJsonObject());
		MetricJson->SetStringField(TEXT("description"), Stat.Description);
		MetricJson->SetNumberField(TEXT("value"), Stat.Value);
		MetricsJson->SetObjectField(MetricName, MetricJson);
	}

	/**
	* This is the format that the Buccaneer server wants for grouped metrics.
	*
	* Grouped metrics could be:
	* - Pixel Streaming peer stats (if we are using one of the Buccaneer Pixel Streaming plugins, each player/peer will have their own stats)
	* - Per-player gameplay stats (if the game is local multiplayer for example and you extended Buccaneer to support that)
	* - Any other type of stats where there are multiple entities each with their own value for the same stat
	*
    * "{MetricName}": {
    *      "description": "{StatDescription}",
    *      "value": [
    *          {
	*             "{GroupId}": {StatValue}
	*          },
    *      ]
    * }
	* Example:
	* "bitrate": {
    *      "description": "The bitrate of the stream",
    *      "value": [
    *			{
	*				"player0": 60
	*			},
	*			{
	*				"player1": 57
	*			},
    *      ]
    * }
    */

	// We need to perform a transformation on our grouped metrics because it does not match the JSON format that the server expects.
	// Specifically, the server is grouped by metric name, whereas our data structure is grouped by group id (e.g. player id).

	// We use this map to build up a JSON object for each stat name (key=stat name, value=JSON object)
	TMap<FString, TSharedPtr<FJsonObject>> GroupMetricsToJsonMap = TMap<FString, TSharedPtr<FJsonObject>>();

	// Build JSON using each of the multi-entry metrics (e.g. per player metrics) that we have stored in the FMetricsCollection
	for (auto const& MetricGroup : GroupedMetrics)
	{
		FString GroupId = MetricGroup.Key;
		const TArray<FBuccaneerMetric>& GroupMetricsArr = MetricGroup.Value;

		// Iterate each stored value within the current multi value metric (e.g. for FPS: player0's fps, player1's fps, etc)
		// and store it the JSON we are building up
		for (const FBuccaneerMetric& Metric : GroupMetricsArr)
		{
			FString MetricName = FormatMetricName(Metric.Name);

			if(!GroupMetricsToJsonMap.Contains(MetricName))
			{
				// Make JSON for the `description` field
				TSharedPtr<FJsonObject> MetricJson = MakeShareable(new FJsonObject());
		 		MetricJson->SetStringField(TEXT("description"), Metric.Description);
				// Make JSON for the `value` field (which is a JSON array)
				TArray<TSharedPtr<FJsonValue>> ValueArray;

				// Note: For these grouped metrics each value is stored in the JSON object like so { "GroupId": value }
				TSharedPtr<FJsonObject> MetricsValueJson = MakeShareable(new FJsonObject());
				MetricsValueJson->SetNumberField(GroupId, Metric.Value);
				ValueArray.Add(MakeShareable(new FJsonValueObject(MetricsValueJson)));

				// Set the `value` array on the MetricJson
				MetricJson->SetArrayField(TEXT("value"), ValueArray);

				// Put the whole MetricJson object we just made into the map
				// so we can add to it as we iterate through the other players
				GroupMetricsToJsonMap.Add(MetricName, MetricJson);
			}
			else
			{
				// We have already created the MetricJson for this MetricName
				// so just need to add to the `value` array
				TSharedPtr<FJsonObject> MetricJson = GroupMetricsToJsonMap[MetricName];
				const TArray<TSharedPtr<FJsonValue>>* ValueArrayPtr;
				if(MetricJson->TryGetArrayField(TEXT("value"), ValueArrayPtr))
				{
					// Copy the existing array so we can modify it
					TArray<TSharedPtr<FJsonValue>> ValueArray = *ValueArrayPtr;

					// Create a new JSON object for this player's value
					TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
					ValueJson->SetNumberField(GroupId, Metric.Value);

					// Add it to the array
					ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));

					// Set the modified array back
					MetricJson->SetArrayField(TEXT("value"), ValueArray);
				}
			}
		}

	}

	// Iterate the `GroupMetricsToJsonMap` and add each stat to the main MetricsJson
    for (const auto& MetricEntry : GroupMetricsToJsonMap)
    {
        const FString& MetricName = MetricEntry.Key;
        const TSharedPtr<FJsonObject>& MetricJson = MetricEntry.Value;
        MetricsJson->SetObjectField(MetricName, MetricJson);
    }

	JsonObject->SetObjectField(TEXT("metrics"), MetricsJson);
	JsonObject->SetNumberField(TEXT("timestamp"), Timestamp);

	return JsonObject;
}
