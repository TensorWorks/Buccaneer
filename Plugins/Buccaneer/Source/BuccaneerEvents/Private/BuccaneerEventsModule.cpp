// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "BuccaneerEventsModule.h"

#include "CoreMinimal.h"
#include "Logging.h"
#include "Dom/JsonObject.h"
#include "IBuccaneerCommonModule.h"
#include "BuccaneerSettings.h"

void FBuccaneerEventsModule::StartupModule()
{    
}

void FBuccaneerEventsModule::ShutdownModule()
{
}

void FBuccaneerEventsModule::EmitEvent(FString Level, FString Event)
{
    if (!UBuccaneerSettings::CVarEnableEvents.GetValueOnAnyThread())
    {
        return;
    }

    UE_LOGFMT(LogBuccaneerEvents, Verbose, "{0}: {1}", Level, Event);

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetField("level", MakeShared<FJsonValueString>(Level));
    JsonObject->SetField("message", MakeShared<FJsonValueString>(Event));

    IBuccaneerCommonModule::Get().SendEvent(JsonObject);
}

IMPLEMENT_MODULE(FBuccaneerEventsModule, BuccaneerEvents)