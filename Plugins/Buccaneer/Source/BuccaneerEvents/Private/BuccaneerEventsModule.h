// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "IBuccaneerEventsModule.h"

class FBuccaneerEventsModule : public IBuccaneerEventsModule
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual void EmitEvent(FString Level, FString Event) override;
};
