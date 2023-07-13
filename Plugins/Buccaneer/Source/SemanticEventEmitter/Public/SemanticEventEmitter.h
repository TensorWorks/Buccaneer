// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(SemanticEventEmitter, Log, All);

class SEMANTICEVENTEMITTER_API FSemanticEventEmitterModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void EmitSemanticEvent(FString Level, FString Event);
    static FSemanticEventEmitterModule *GetModule();

private:
    static FSemanticEventEmitterModule *SemanticEmitterModule;
};
