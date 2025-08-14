// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "IBuccaneerCommonModule.h"
#include "IBuccaneer4PixelStreaming2Module.h"
#include "Modules/ModuleManager.h"
#include "PixelStreaming2Delegates.h"

class FBuccaneer4PixelStreaming2Module : public IBuccaneer4PixelStreaming2Module
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void ConsumeStat(FString PlayerId, FName StatName, float StatValue);
	
private:
	double LoggingStart;

    TSharedPtr<FJsonObject> JsonObject;
};
