// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "Buccaneer4PixelStreaming2Settings.generated.h"

// Config loaded/saved to an .ini file.
// It is also exposed through the plugin settings page in editor.
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Buccaneer4PixelStreaming2"))
class BUCCANEER4PIXELSTREAMING2_API UBuccaneer4PixelStreaming2Settings : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual ~UBuccaneer4PixelStreaming2Settings() = default;

public:
    static TAutoConsoleVariable<bool> CVarEnabled;
    UPROPERTY(config, EditAnywhere, Category = "Buccaneer4PixelStreaming2", meta = (
		DisplayName = "Enabled",
		ToolTip = "Enables the collection of Pixel Streaming 2 performance metrics"
		))
    bool Enabled = true;

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

private:
	void SetCVarAndPropertyFromValue(const FString& CVarName, FProperty* Property, const FString& Value);
	void SetCVarFromProperty(const FString& CVarName, FProperty* Property);

	void InitializeCVarsFromProperties();
	void ValidateCommandLineArgs();
	void ParseCommandlineArgs();
};