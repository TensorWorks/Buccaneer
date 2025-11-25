// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "IBuccaneerCommonModule.h"

struct FMetricsCollection;

class FBuccaneerCommonModule : public IBuccaneerCommonModule
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual FReadyEvent& OnReady() override;
    virtual bool IsReady() override;
    virtual void SendMetrics(const FMetricsCollection& StatsCollection) override;
    virtual void SendEvent(TSharedPtr<FJsonObject> JsonObject) override;

private:
    bool bModuleReady = false;
    FReadyEvent ReadyEvent;

private:
    void SendHTTP(FString URL, TSharedPtr<FJsonObject> JsonObject);
    void WriteJSON(FString FileName, TSharedPtr<FJsonObject> JsonObject);
    void FormatMetadata(IConsoleVariable* Var);

    TSharedPtr<FJsonObject> MetadataJson = MakeShareable(new FJsonObject());
    FString CachedJSONOutputFileName;
};
