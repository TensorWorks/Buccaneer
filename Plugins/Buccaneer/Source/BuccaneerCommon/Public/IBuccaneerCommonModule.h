// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Dom/JsonObject.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "BuccaneerMetrics.h"

class BUCCANEERCOMMON_API IBuccaneerCommonModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface.
	 * Beware calling this during the shutdown phase, though. Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IBuccaneerCommonModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IBuccaneerCommonModule>("BuccaneerCommon");
	}

	/**
	 * Checks to see if this module is loaded.
	 *
	 * @return True if the module is loaded.
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BuccaneerCommon");
	}

	/**
	 * Event fired when internal streamer is initialized and the methods on this module are ready for use.
	 */
	DECLARE_EVENT_OneParam(IBuccaneerCommonModule, FReadyEvent, IBuccaneerCommonModule&);

	/**
	 * A getter for the OnReady event. Intent is for users to call IBuccaneerCommonModule::Get().OnReady().AddXXX.
	 * @return The bindable OnReady event.
	 */
	virtual FReadyEvent& OnReady() = 0;

	/**
	 * Is the BuccaneerCommon module actually ready to use? Is the streamer created.
	 * @return True if BuccaneerCommon module methods are ready for use.
	 */
	virtual bool IsReady() = 0;

	/**
	 *
	 */
	virtual void SendMetrics(const FMetricsCollection& StatsCollection) = 0;

	/**
	 *
	 */
	virtual void SendEvent(TSharedPtr<FJsonObject> JsonObject) = 0;
};