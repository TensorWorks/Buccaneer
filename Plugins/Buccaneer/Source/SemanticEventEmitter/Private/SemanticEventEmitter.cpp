// Copyright Epic Games, Inc. All Rights Reserved.

#include "SemanticEventEmitter.h"
#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Logging/LogMacros.h"
#include "Dom/JsonObject.h"
#include "BuccaneerCommon.h"

#define LOCTEXT_NAMESPACE "SemanticEventEmitterModule"

DEFINE_LOG_CATEGORY(SemanticEventEmitter);

FSemanticEventEmitterModule *FSemanticEventEmitterModule::SemanticEmitterModule = nullptr;

void FSemanticEventEmitterModule::StartupModule()
{
    
}

void FSemanticEventEmitterModule::ShutdownModule()
{
}

void FSemanticEventEmitterModule::EmitSemanticEvent(FString Level, FString Event)
{
    if (!FBuccaneerCommonModule::GetModule()->CVarBuccaneerEnableEvents->GetBool())
    {
        return;
    }

    UE_LOG(SemanticEventEmitter, Verbose, TEXT("%s: %s"), *Level, *Event);

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetField("level", MakeShared<FJsonValueString>((TEXT("%s"), *Level)));
    JsonObject->SetField("message", MakeShared<FJsonValueString>((TEXT("%s"), *Event)));

    FBuccaneerCommonModule::GetModule()->SendEvent(JsonObject);
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