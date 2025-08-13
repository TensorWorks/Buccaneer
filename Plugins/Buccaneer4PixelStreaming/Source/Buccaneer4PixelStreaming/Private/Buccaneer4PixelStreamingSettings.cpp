// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "Buccaneer4PixelStreamingSettings.h"

#include "BuccaneerSettings.h"
#include "Logging.h"
#include "Misc/CommandLine.h"
#include "UObject/ReflectedTypeAccessors.h"

static const TSet<TPair<FString, FString>> GetCmdArg = {
	{ "Buccaneer4PixelStreaming.EnableStats", "Enabled" }
};

TAutoConsoleVariable<bool> UBuccaneer4PixelStreamingSettings::CVarEnabled(
	TEXT("Buccaneer4PixelStreaming.EnableStats"),
	true,
	TEXT("Enables the collection and logging of Pixel Streaming stats with Buccaneer (default: true)"),
	ECVF_Default);

FName UBuccaneer4PixelStreamingSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

#if WITH_EDITOR
FText UBuccaneer4PixelStreamingSettings::GetSectionText() const
{
	return NSLOCTEXT("Buccaneer4PixelStreamingPlugin", "Buccaneer4PixelStreamingSettingsSection", "Buccaneer4PixelStreaming");
}

void UBuccaneer4PixelStreamingSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FString PropertyName = PropertyChangedEvent.Property->GetNameCPP();

	FString CVarName;
	if (CVarName = Util::FindCVarFromProperty(GetCmdArg, PropertyName); !CVarName.IsEmpty())
	{
        SetCVarFromProperty(CVarName, PropertyChangedEvent.Property);
	}
}
#endif

void UBuccaneer4PixelStreamingSettings::SetCVarAndPropertyFromValue(const FString& CVarName, FProperty* Property, const FString& Value)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (!CVar)
	{
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Warning, "Failed to find CVar: {0}", CVarName);
		return;
	}

	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property); ByteProperty != NULL && ByteProperty->Enum != NULL)
	{
		CVar->Set(FCString::Atoi(*Value), ECVF_SetByCommandline);
		ByteProperty->SetPropertyValue_InContainer(this, FCString::Atoi(*Value));
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atoi(*Value));
	}
	else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		int64 EnumIndex = EnumProperty->GetEnum()->GetIndexByNameString(Value.Replace(TEXT("_"), TEXT("")));
		if (EnumIndex != INDEX_NONE)
		{
			CVar->Set(*EnumProperty->GetEnum()->GetNameStringByIndex(EnumIndex), ECVF_SetByCommandline);

			FNumericProperty* UnderlyingProp = EnumProperty->GetUnderlyingProperty();
			int64*			  PropertyAddress = EnumProperty->ContainerPtrToValuePtr<int64>(this);
			*PropertyAddress = EnumProperty->GetEnum()->GetValueByIndex(EnumIndex);

			UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), EnumProperty->GetEnum()->GetNameStringByIndex(EnumIndex));
		}
		else
		{
			UE_LOGFMT(LogBuccaneer4PixelStreaming, Warning, "{0} is not a valid enum value for {1}", Value, EnumProperty->GetEnum()->CppType);
		}
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		bool bValue = false;
		if (Value.Equals(FString(TEXT("true")), ESearchCase::IgnoreCase))
		{
			bValue = true;
		}
		else if (Value.Equals(FString(TEXT("false")), ESearchCase::IgnoreCase))
		{
			bValue = false;
		}
		CVar->Set(bValue, ECVF_SetByCommandline);
		BoolProperty->SetPropertyValue_InContainer(this, bValue);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), bValue);
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		CVar->Set(FCString::Atoi(*Value), ECVF_SetByCommandline);
		IntProperty->SetPropertyValue_InContainer(this, FCString::Atoi(*Value));
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atoi(*Value));
	}
	else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		CVar->Set(FCString::Atof(*Value), ECVF_SetByCommandline);
		FloatProperty->SetPropertyValue_InContainer(this, FCString::Atof(*Value));
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atof(*Value));
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		CVar->Set(*Value, ECVF_SetByCommandline);
		StringProperty->SetPropertyValue_InContainer(this, *Value);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		CVar->Set(*Value, ECVF_SetByCommandline);
		NameProperty->SetPropertyValue_InContainer(this, FName(*Value));
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		// TODO (william.belcher): Only FString array properties are currently supported
		CVar->Set(*Value, ECVF_SetByCommandline);

		TArray<FString> StringArray;
		Value.ParseIntoArray(StringArray, TEXT(","), true);

		TArray<FString>& Array = *ArrayProperty->ContainerPtrToValuePtr<TArray<FString>>(this);
		Array = StringArray;

		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
}

void UBuccaneer4PixelStreamingSettings::SetCVarFromProperty(const FString& CVarName, FProperty* Property)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (!CVar)
	{
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Warning, "Failed to find CVar: {0}", CVarName);
		return;
	}

	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property); ByteProperty != NULL && ByteProperty->Enum != NULL)
	{
		CVar->Set(ByteProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, ByteProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		void* PropertyAddress = EnumProperty->ContainerPtrToValuePtr<void>(this);
		int64 CurrentValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(PropertyAddress);
		CVar->Set(*EnumProperty->GetEnum()->GetNameStringByValue(CurrentValue), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, EnumProperty->GetEnum()->GetNameStringByValue(CurrentValue), Property->GetNameCPP());
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		CVar->Set(BoolProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, BoolProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		CVar->Set(IntProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, IntProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		CVar->Set(FloatProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, FloatProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		CVar->Set(*StringProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, StringProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		CVar->Set(*NameProperty->GetPropertyValue_InContainer(this).ToString(), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, NameProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		// TODO (william.belcher): Only FString array properties are currently supported
		TArray<FString> Array = *ArrayProperty->ContainerPtrToValuePtr<TArray<FString>>(this);
		CVar->Set(*FString::Join(Array, TEXT(",")), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, FString::Join(Array, TEXT(",")), Property->GetNameCPP());
	}
}

void UBuccaneer4PixelStreamingSettings::InitializeCVarsFromProperties()
{
	UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Initializing CVars from ini");
	for (FProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		FString CVarName;
		if (CVarName = Util::FindCVarFromProperty(GetCmdArg, Property->GetNameCPP()); !CVarName.IsEmpty())
		{
			SetCVarFromProperty(CVarName, Property);
			continue;
		}
	}
}

void UBuccaneer4PixelStreamingSettings::ValidateCommandLineArgs()
{
	FString CommandLine = FCommandLine::Get();

	TArray<FString> CommandArray;
	CommandLine.ParseIntoArray(CommandArray, TEXT(" "), true);

	for (FString Command : CommandArray)
	{
		Command.RemoveFromStart(TEXT("-"));
		if (!Command.StartsWith("Buccaneer4PixelStreaming"))
		{
			continue;
		}

		// Get the pure command line arg from an arg that contains an '=', eg BuccaneerURL=
		FString CurrentCommandLineArg = Command;
		if (Command.Contains("="))
		{
			Command.Split(TEXT("="), &CurrentCommandLineArg, nullptr);
		}

		bool bValidArg = false;
		for (const TPair<FString, FString>& Pair : GetCmdArg)
		{
			FString ValidCommandLineArg = Util::ConsoleVariableToCommandArgParam(Pair.Key);
			if (CurrentCommandLineArg == ValidCommandLineArg)
			{
				bValidArg = true;
				break;
			}
		}

		if (!bValidArg)
		{
			UE_LOGFMT(LogBuccaneer4PixelStreaming, Warning, "Unknown Buccaneer4PixelStreaming command line arg: {0}", CurrentCommandLineArg);
		}
	}
}

void UBuccaneer4PixelStreamingSettings::ParseCommandlineArgs()
{
	UE_LOGFMT(LogBuccaneer4PixelStreaming, Verbose, "Updating CVars and properties with command line args");
	for (const TPair<FString, FString>& Pair : GetCmdArg)
	{
		FString CVarString = Pair.Key;
		FString PropertyName = Pair.Value;

		FProperty* Property = GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property || !Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		// Handle a directly parsable commandline
		FString ConsoleString;
		if (FParse::Value(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgValue(CVarString), ConsoleString))
		{
			SetCVarAndPropertyFromValue(CVarString, Property, ConsoleString);
		}
		else if (FParse::Param(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgParam(CVarString)))
		{
			SetCVarAndPropertyFromValue(CVarString, Property, TEXT("true"));
		}
	}
}

void UBuccaneer4PixelStreamingSettings::PostInitProperties()
{
	Super::PostInitProperties();

	UE_LOGFMT(LogBuccaneer4PixelStreaming, Log, "Initialising Buccaneer4PixelStreaming settings.");

	// Set all the CVars to reflect the state of the ini
	InitializeCVarsFromProperties();

	// Validate command line args to log if they're invalid
	ValidateCommandLineArgs();

	// Update CVars and properties based on command line args
	ParseCommandlineArgs();
}