#include "BuccaneerMetrics.h"

TSharedPtr<FJsonObject> FMetricsCollection::ToJsonNested() const
{
	// This function makes a JSON object that follows this structure:
	/*
	*
	* {
	*   "metrics": 
	*	{
	*		"fps": 
	*		{
	*			"description": "The framerate of the game"
	*			"value": 60
	*		}
	*		"player0":
	*		{
	*			"bitrate":
	*			{
	*				"description": "This peer's streaming bitrate"
	*				"value": 1000
	*			}
	*		} 
	*	}
	* }
	*/

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> MetricsJson = MakeShareable(new FJsonObject());

	// Build JSON object using the global metrics we have stored in the FMetricsCollection
	for (const FBuccaneerMetric &Stat : Metrics)
	{
		TSharedPtr<FJsonObject> StatJson = MakeShareable(new FJsonObject());
		StatJson->SetStringField(TEXT("description"), Stat.Description);
		StatJson->SetNumberField(TEXT("value"), Stat.Value);
		MetricsJson->SetObjectField(Stat.Name, StatJson);
	}

	// This is what it wants for Pixel Streaming:
	/**
    * "{StatName}": {
    *      "description": "{StatDescription}",
    *      "value": [
    *          "{PlayerId}": {StatValue}
    *      ]
    * }
	* Example:
	* "fps": {
    *      "description": "The framerate of the game",
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

	// Basically just need to do this logic from original Buc:

	// From: Private/Buccaneer4PixelStreaming.cpp
	// TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
	// ValueJson->SetField(*PlayerId, MakeShared<FJsonValueNumber>(StatValue));
	// TArray<TSharedPtr<FJsonValue>> ValueArray;
	// ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));
	// NewMetricJson->SetArrayField((TEXT("value")), ValueArray);

	// Build JSON using each of the player metrics that we have stored in the FMetricsCollection
	for (auto const& PlayerEntry : PlayerMetrics)
	{
		FString PlayerId = PlayerEntry.Key;
		const TArray<FBuccaneerMetric>& PlayerStats = PlayerEntry.Value;

		TSharedPtr<FJsonObject> SinglePlayerMetricsJson = MakeShareable(new FJsonObject());
		for (const FBuccaneerMetric &Stat : PlayerStats)
		{
			TSharedPtr<FJsonObject> StatJson = MakeShareable(new FJsonObject());
			StatJson->SetStringField(TEXT("description"), Stat.Description);
			// todo: This needs to be changed from being set to a "number" to instead be set using `SetArrayField` as above
			StatJson->SetNumberField(TEXT("value"), Stat.Value);
			SinglePlayerMetricsJson->SetObjectField(Stat.Name, StatJson);
		}
		MetricsJson->SetObjectField(PlayerId, SinglePlayerMetricsJson);
	}

	JsonObject->SetObjectField(TEXT("metrics"), MetricsJson);
	JsonObject->SetNumberField(TEXT("timestamp"), Timestamp);

	return JsonObject;
}

TSharedPtr<FJsonObject> FMetricsCollection::ToJsonUnnested() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	for (const FBuccaneerMetric &Stat : Metrics)
	{
		JsonObject->SetField(Stat.Name, MakeShared<FJsonValueNumber>(Stat.Value));
	}

	// Add PlayerMetrics (unnested, so composite name)
	for (auto const& PlayerEntry : PlayerMetrics)
	{
		FString PlayerId = PlayerEntry.Key;
		const TArray<FBuccaneerMetric>& PlayerStats = PlayerEntry.Value;

		for (const FBuccaneerMetric &Stat : PlayerStats)
		{
			FString CompositeName = FString::Printf(TEXT("%s_%s"), *PlayerId, *Stat.Name);
			JsonObject->SetField(CompositeName, MakeShared<FJsonValueNumber>(Stat.Value));
		}
	}

	JsonObject->SetField(TEXT("timestamp"), MakeShared<FJsonValueNumber>(Timestamp));

	return JsonObject;
}
