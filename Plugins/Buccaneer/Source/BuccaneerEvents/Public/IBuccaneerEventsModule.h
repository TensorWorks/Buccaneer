// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class BUCCANEEREVENTS_API IBuccaneerEventsModule : public IModuleInterface
{
public:
    /**
	 * Singleton-like access to this module's interface.
	 * Beware calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
    static inline IBuccaneerEventsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IBuccaneerEventsModule>("BuccaneerEvents");
	}

    /**
	 * Checks to see if this module is loaded.
	 *
	 * @return True if the module is loaded.
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BuccaneerEvents");
	}

    /**
     * 
     */
    virtual void EmitEvent(FString Level, FString Event) = 0;
};
