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

FSemanticEventEmitterModule *FSemanticEventEmitterModule::SemanticEmitterModule = nullptr;

void FSemanticEventEmitterModule::StartupModule()
{
    JsonObject = MakeShareable(new FJsonObject());
    bIsReady = true;
}

void FSemanticEventEmitterModule::ShutdownModule()
{
}

void FSemanticEventEmitterModule::EmitSemanticEvent(FString Level, FString Event)
{
    if (!FBuccaneerCommonModule::GetModule()->CVarBuccaneerEnableEvents->GetBool() || !bIsReady)
    {
        return;
    }

    UE_LOG(SemanticEventEmitter, Log, TEXT("%s: %s"), *Level, *Event);
    FBuccaneerCommonModule *Module = FBuccaneerCommonModule::GetModule();
    if (Module && Module->CVarBuccaneerEnableStats->GetBool())
    {
        JsonObject->SetField("level", MakeShared<FJsonValueString>((TEXT("%s"), *Level)));
        JsonObject->SetField("message", MakeShared<FJsonValueString>((TEXT("%s"), *Event)));

        Module->SendEvent(JsonObject);
    }
}

FSemanticEventEmitterModule *FSemanticEventEmitterModule::GetModule()
{
    if (SemanticEmitterModule)
    {
        return SemanticEmitterModule;
    }
    FSemanticEventEmitterModule *Module = FModuleManager::Get().GetModulePtr<FSemanticEventEmitterModule>("SemanticEventEmitter");
    if (Module)
    {
        SemanticEmitterModule = Module;
    }
    return SemanticEmitterModule;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSemanticEventEmitterModule, SemanticEventEmitter)