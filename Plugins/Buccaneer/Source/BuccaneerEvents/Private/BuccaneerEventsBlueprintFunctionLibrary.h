// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IBuccaneerEventsModule.h"

#include "BuccaneerEventsBlueprintFunctionLibrary.generated.h"

UCLASS()
class BUCCANEEREVENTS_API UBuccaneerEventsBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Buccaneer")
    static void EmitEvent(FString Level, FString Event);    
};
