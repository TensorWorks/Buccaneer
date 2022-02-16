// Copyright Epic Games, Inc. All Rights Reserved.

#include "SemanticEventBlueprintFunctionLibrary.h"


void USemanticEventEmitterBlueprintLibrary::EmitSemanticEvent(FString Level, FString Event) 
{
    FSemanticEventEmitterModule* Module = FSemanticEventEmitterModule::GetModule();
    if(Module)
    {
        Module->EmitSemanticEvent(Level, Event);
    }
}