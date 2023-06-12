#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"

#include <memory.h>
#include <string>

DECLARE_MULTICAST_DELEGATE(FOnSetupComplete);

DECLARE_LOG_CATEGORY_EXTERN(BuccaneerCommon, Log, All);

class BUCCANEERCOMMON_API FBuccaneerCommonModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FBuccaneerCommonModule *GetModule();

    void SendStats(TSharedPtr<FJsonObject> JsonObject);
    void SendEvent(TSharedPtr<FJsonObject> JsonObject);

    FOnSetupComplete SetupComplete;

    IConsoleVariable *CVarBuccaneerEnableStats;
    IConsoleVariable *CVarBuccaneerEnableEvents;

private:
    void Setup();
    void ParseCommandLineOption(const TCHAR *Match, IConsoleVariable *CVar);

    void SendHTTP(FString URL, TSharedPtr<FJsonObject> JsonObject);

    FString BuccaneerURL;
    FString InstanceID;
    TSharedPtr<FJsonObject> MetadataJson;

    static FBuccaneerCommonModule *BuccaneerCommonModule;
};
