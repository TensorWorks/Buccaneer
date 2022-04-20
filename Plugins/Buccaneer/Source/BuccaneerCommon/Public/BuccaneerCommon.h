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

	static FBuccaneerCommonModule* GetModule();

    void SendHTTP(FString URL, TSharedPtr<FJsonObject> JsonObject);
	void SendHTTPWithResponse(FString URL, TSharedPtr<FJsonObject> JsonObject);
	void ParseCommandLineOption(const TCHAR* Match, IConsoleVariable* CVar);

	FOnSetupComplete SetupComplete;
	
	FString ID;
	FString StatsEmitterURL;
	FString EventEmitterURL;

	bool bSetupCalled = false;
	
	TSharedPtr<FJsonObject> MetadataJson;

	IConsoleVariable* CVarBuccaneerEnableStats;
	IConsoleVariable* CVarBuccaneerEnableEvents;

private:

	void Setup();	
	void RegisterMetadata(FString Key, FString Value);
	void HandleResponse(FString ResponseString);

	static FBuccaneerCommonModule* BuccaneerCommonModule;	
};


