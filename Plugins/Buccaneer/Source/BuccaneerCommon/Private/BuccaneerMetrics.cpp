#include "BuccaneerMetrics.h"

TSharedPtr<FJsonObject> FMetricsCollection::ToJsonNested() const
{

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

	// This is the format that Buc wants from Pixel Streaming 1/2:
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

	// Build JSON using each of the player metrics that we have stored in the FMetricsCollection
	for (auto const& PlayerEntry : PlayerMetrics)
	{
		FString PlayerId = PlayerEntry.Key;
		const TArray<FBuccaneerMetric>& PlayerStats = PlayerEntry.Value;

		TMap<FString, TSharedPtr<FJsonObject>> PlayerStatsJsonMap = TMap<FString, TSharedPtr<FJsonObject>>();

		// Iterate each stat within the individual player's stats
		// and extract the value for each and store it on MetricsJson
		for (const FBuccaneerMetric &Stat : PlayerStats)
		{
			const FString& OriginalStatName = Stat.Name;
			// Replace any hyphens from the stat name with underscores to ensure valid prometheus metric names
			FString StatName = OriginalStatName;
			StatName.ReplaceInline(TEXT("-"), TEXT("_"));

			if(!PlayerStatsJsonMap.Contains(StatName))
			{
				// Make JSON for the `description` field
				TSharedPtr<FJsonObject> StatJson = MakeShareable(new FJsonObject());
		 		StatJson->SetStringField(TEXT("description"), Stat.Description);
				// Make JSON for the `value` field (which is a JSON array)
				TArray<TSharedPtr<FJsonValue>> ValueArray;

				// Note each value is stored in the JSON array as { "playerId": value }
				TSharedPtr<FJsonObject> PlayerStatJson = MakeShareable(new FJsonObject());
				PlayerStatJson->SetNumberField(PlayerId, Stat.Value);
				ValueArray.Add(MakeShareable(new FJsonValueObject(PlayerStatJson)));

				// Set the `value` array on the StatJson
				StatJson->SetArrayField(TEXT("value"), ValueArray);

				// Put the whole StatJson object we just made into the map
				// so we can add to it as we iterate through the other players
				PlayerStatsJsonMap.Add(StatName, StatJson);
			}
			else
			{
				// We have already created the StatJson for this stat name
				// so just need to add to the `value` array
				TSharedPtr<FJsonObject> StatJson = PlayerStatsJsonMap[StatName];
				const TArray<TSharedPtr<FJsonValue>>* ValueArrayPtr;
				if(StatJson->TryGetArrayField(TEXT("value"), ValueArrayPtr))
				{
					// Copy the existing array so we can modify it
					TArray<TSharedPtr<FJsonValue>> ValueArray = *ValueArrayPtr;

					// Create a new JSON object for this player's value
					TSharedPtr<FJsonObject> ValueJson = MakeShareable(new FJsonObject());
					ValueJson->SetNumberField(PlayerId, Stat.Value);

					// Add it to the array
					ValueArray.Add(MakeShareable(new FJsonValueObject(ValueJson)));

					// Set the modified array back
					StatJson->SetArrayField(TEXT("value"), ValueArray);
				}
			}
		}

		// Iterate the `PlayerStatsJsonMap` and add each stat to the main MetricsJson
		for(auto const& StatEntry : PlayerStatsJsonMap)
		{
			FString StatName = StatEntry.Key;
			TSharedPtr<FJsonObject> StatJson = StatEntry.Value;
			MetricsJson->SetObjectField(StatName, StatJson);
		}
	}

	JsonObject->SetObjectField(TEXT("metrics"), MetricsJson);
	JsonObject->SetNumberField(TEXT("timestamp"), Timestamp);

	return JsonObject;
}
