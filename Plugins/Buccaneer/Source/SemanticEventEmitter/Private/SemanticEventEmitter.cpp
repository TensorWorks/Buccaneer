// Copyright Epic Games, Inc. All Rights Reserved.

#include "SemanticEventEmitter.h"
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Logging/LogMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "BuccaneerCommon.h"

#define LOCTEXT_NAMESPACE "SemanticEventEmitterModule"

DEFINE_LOG_CATEGORY(SemanticEventEmitter);

FSemanticEventEmitterModule* FSemanticEventEmitterModule::SemanticEmitterModule = nullptr;

void FSemanticEventEmitterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBuccaneerCommonModule* BuccaneerCommonModule = FBuccaneerCommonModule::GetModule();
    if(!BuccaneerCommonModule->bEnableEvents)
    {
        return;
    }
    // Add a callback to setup this module when Buccaneer Common has finished setting up
	BuccaneerCommonModule->SetupComplete.AddRaw(this, &FSemanticEventEmitterModule::Setup);
	
}

void FSemanticEventEmitterModule::Setup()
{
	JsonObject = MakeShareable(new FJsonObject());
	bIsEnabled = true;
}

void FSemanticEventEmitterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FSemanticEventEmitterModule::EmitSemanticEvent(FString Level, FString Event)
{
	if(bIsEnabled)
	{
		UE_LOG(SemanticEventEmitter, Log, TEXT("%s: %s"), *Level, *Event);	
		PushEventHTTP(Level, Event);
	}
	else
	{
		UE_LOG(SemanticEventEmitter, Log, TEXT("SemanticEventEmitterModule isn't setup"));	
	}
	
}

FSemanticEventEmitterModule* FSemanticEventEmitterModule::GetModule()
{
    if(SemanticEmitterModule) 
    {
        return SemanticEmitterModule;
    }
    FSemanticEventEmitterModule* Module = FModuleManager::Get().GetModulePtr<FSemanticEventEmitterModule>("SemanticEventEmitter");
    if(Module)
    {
        SemanticEmitterModule = Module;
    }
    return SemanticEmitterModule;
}

void FSemanticEventEmitterModule::PushEventHTTP(FString Level, FString Event) {
	FBuccaneerCommonModule* Module = FBuccaneerCommonModule::GetModule();
    if(Module)
    {
		JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *Module->ID)));
		JsonObject->SetField("level", MakeShared<FJsonValueString>((TEXT("%s"), *Level)));
		JsonObject->SetField("message", MakeShared<FJsonValueString>((TEXT("%s"), *Event)));

        Module->SendHTTP(Module->EventEmitterURL, JsonObject);
    }
	else
	{
		UE_LOG(SemanticEventEmitter, Error, TEXT("BuccaneerCommonModule not loaded"));
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSemanticEventEmitterModule, SemanticEventEmitter)