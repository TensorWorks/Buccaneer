// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "BuccaneerSettings.generated.h"

namespace Util
{
    BUCCANEERCOMMON_API FString ConsoleVariableToCommandArgValue(const FString InCVarName);

	BUCCANEERCOMMON_API FString ConsoleVariableToCommandArgParam(const FString InCVarName);

    BUCCANEERCOMMON_API FString FindCVarFromProperty(const TSet<TPair<FString, FString>> Set, const FString& Value);
}

// Config loaded/saved to an .ini file.
// It is also exposed through the plugin settings page in editor.
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Buccaneer"))
class BUCCANEERCOMMON_API UBuccaneerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual ~UBuccaneerSettings();

public:
    static TAutoConsoleVariable<FString> CVarURL;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "URL",
		ToolTip = "URL to send stats and events to. This should be a URL to a Buccaneer Server"
		))
    FString URL;

	static TAutoConsoleVariable<FString> CVarID;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "ID",
		ToolTip = "ID to identify this instance. Auto-generates a short GUID if not provided"
		))
    FString ID;

    static TAutoConsoleVariable<bool> CVarEnableStats;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "Enable Stats",
		ToolTip = "Enables the collection of performance metrics"
		))
    bool EnableStats = true;

    static TAutoConsoleVariable<bool> CVarEnableEvents;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "Enable Events",
		ToolTip = "Enables the collection of semantic events"
		))
    bool EnableEvents = true;

	static TAutoConsoleVariable<FString> CVarMetadata;
	UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "Metadata",
		ToolTip = "Key:Value pairs of metadata to send with this instance",
		ForceInlineRow
		))
    TMap<FString, FString> Metadata;

	static TMap<FString, FString> GetMetadata();

	static TAutoConsoleVariable<float> CVarReportingInterval;
	UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
		DisplayName = "Reporting Interval (seconds)",
		ToolTip = "The interval at which to report performance metrics. <= 0 disables reporting"
		))
    float ReportingInterval = 1.0f;

    static TAutoConsoleVariable<bool> CVarEnableJSONOutput;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
        DisplayName = "Enable JSON Output",
        ToolTip = "Enables writing stats and events to a JSON file"
        ))
    bool EnableJSONOutput = false;

    static TAutoConsoleVariable<FString> CVarJSONOutputDirectory;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
        DisplayName = "JSON Output Directory",
        ToolTip = "The directory to write JSON files to"
        ))
    FString JSONOutputDirectory = FPaths::ProjectLogDir();

    static TAutoConsoleVariable<FString> CVarJSONOutputFile;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer", meta = (
        DisplayName = "JSON Output File",
        ToolTip = "The filename for JSON output (default: <BuccaneerID>_Stats.json)"
        ))
    FString JSONOutputFile;

	// Begin UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End UDeveloperSettings Interface

	// Begin UObject Interface
	virtual void PostInitProperties() override;
	// End UObject Interface

	struct FDelegates
	{
		DECLARE_TS_MULTICAST_DELEGATE_OneParam(FOnMetadataChanged, IConsoleVariable*);
		FOnMetadataChanged OnMetadataChanged;
	};

	static FDelegates* Delegates();

private:
	void SetCVarAndPropertyFromValue(const FString& CVarName, FProperty* Property, const FString& Value);
	void SetCVarFromProperty(const FString& CVarName, FProperty* Property);

	void InitializeCVarsFromProperties();
	void ValidateCommandLineArgs();
	void ParseCommandlineArgs();
	void ParseLegacyCommandlineArgs();

	static FDelegates* DelegateSingleton;

};