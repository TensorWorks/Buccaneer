// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "BuccaneerEventsBlueprintFunctionLibrary.h"

void UBuccaneerEventsBlueprintFunctionLibrary::EmitEvent(FString Level, FString Event) 
{
    IBuccaneerEventsModule::Get().EmitEvent(Level, Event);
}