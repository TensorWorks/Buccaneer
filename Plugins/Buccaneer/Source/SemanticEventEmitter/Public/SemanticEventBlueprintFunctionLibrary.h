// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SemanticEventEmitter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SemanticEventBlueprintFunctionLibrary.generated.h"

UCLASS()
class SEMANTICEVENTEMITTER_API USemanticEventEmitterBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Buccaneer")
    static void EmitSemanticEvent(FString Level, FString Event);    
};
