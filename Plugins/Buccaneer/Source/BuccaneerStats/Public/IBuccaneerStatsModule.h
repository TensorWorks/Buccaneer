// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Dom/JsonObject.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class BUCCANEERSTATS_API IBuccaneerStatsModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface.
	 * Beware calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IBuccaneerStatsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IBuccaneerStatsModule>("BuccaneerStats");
	}

    /**
	 * Checks to see if this module is loaded.
	 *
	 * @return True if the module is loaded.
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BuccaneerStats");
	}

	/*
	* Get the timestamp for the purposes of stats reporting (so we all use the same clock/timekeeping system)
	*/
	static int64 GetStatsTimestamp()
	{
		// Return platform timestamp in milliseconds
		return FMath::RoundToInt64((FPlatformTime::Seconds()) * 1000);
	}
};