// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "BuccaneerSettings.h"

#include "Logging.h"
#include "Misc/CommandLine.h"
#include "UObject/ReflectedTypeAccessors.h"

namespace Util
{
    FString ConsoleVariableToCommandArgValue(const FString InCVarName)
	{
		// CVars are . deliminated by section. To get their equivilent commandline arg for parsing
		// we need to remove the . and add a "="
		return InCVarName.Replace(TEXT("."), TEXT("")).Append(TEXT("="));
	}

	FString ConsoleVariableToCommandArgParam(const FString InCVarName)
	{
		// CVars are . deliminated by section. To get their equivilent commandline arg parameter, we need to to remove the .
		return InCVarName.Replace(TEXT("."), TEXT(""));
	}

    FString FindCVarFromProperty(const TSet<TPair<FString, FString>> Set, const FString& Value)
	{
		for (const TPair<FString, FString>& Pair : Set)
		{
			if (Pair.Value == Value)
			{
				return Pair.Key;
			}
		}

		return "";
	}

    void SetMetadataCVarFromProperty(UObject* This, FProperty* Property)
    {
        if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
        {
            FString CVarString = "";

            TMap<FString, FString>& Map = *MapProperty->ContainerPtrToValuePtr<TMap<FString, FString>>(This);
            for (const TPair<FString, FString>& Pair : Map)
            {
                if (Pair.Key.IsEmpty() || Pair.Value.IsEmpty())
                {
                    continue;
                }

                CVarString += FString::Printf(TEXT("%s:%s;"), *Pair.Key, *Pair.Value);
            }

    	    UBuccaneerSettings::CVarMetadata->Set(*CVarString, ECVF_SetByProjectSetting);

            UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [Buccaneer.Metadata] to [\"{1}\"] from Property [Metadata]", CVarString);
        }
    }

    void SetMetadataCVarAndPropertyFromValue(UObject* This, FProperty* Property, const FString& CmdValue)
    {
	    UBuccaneerSettings::CVarMetadata->Set(*CmdValue, ECVF_SetByCommandline);

        if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
        {
            TMap<FString, FString>& Map = *MapProperty->ContainerPtrToValuePtr<TMap<FString, FString>>(This);

            TArray<FString> Pairs;
            CmdValue.ParseIntoArray(Pairs, TEXT(";"), true);
            for (FString Pair : Pairs)
            {
                if (Pair.IsEmpty())
                {
                    continue;
                }

                FString Key, Value;
                Pair.Split(TEXT(":"), &Key, &Value);
                if(!Key.IsEmpty() && Value.IsEmpty())
                {
                    Map.Add(Key, Value);
                }
            }
        }

        UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [Buccaneer.Metadata] and Property [Metadata] to [{0}] from command line", CmdValue);
    }
}

static const TSet<TPair<FString, FString>> GetCmdArg = {
    { "Buccaneer.URL", "URL" },
    { "Buccaneer.ID", "ID" },
	{ "Buccaneer.EnableStats", "EnableStats" },
	{ "Buccaneer.EnableEvents", "EnableEvents" },
    { "Buccaneer.Metadata", "Metadata" },
	{ "Buccaneer.ReportingInterval", "ReportingInterval" }
};

// Map a legacy cvar to its new property
static const TSet<TPair<FString, FString>> GetLegacyCmdArg = {
	{ "Buccaneer.IP", "URL" },   // Moved to URL
	{ "Buccaneer.Port", "URL" }  // Moved to URL
};

TAutoConsoleVariable<FString> UBuccaneerSettings::CVarURL(
	TEXT("Buccaneer.URL"),
	TEXT(""),
	TEXT("URL to send stats and events to. This should be a URL to a Buccaneer Server"),
	ECVF_Default);

TAutoConsoleVariable<FString> UBuccaneerSettings::CVarID(
	TEXT("Buccaneer.ID"),
	TEXT(""),
	TEXT("ID to identify this instance. Defaults to a new GUID"),
	ECVF_Default);

TAutoConsoleVariable<bool> UBuccaneerSettings::CVarEnableStats(
	TEXT("Buccaneer.EnableStats"),
	true,
	TEXT("Enables the collection of performance metrics (default: true)"),
	ECVF_Default);

TAutoConsoleVariable<bool> UBuccaneerSettings::CVarEnableEvents(
	TEXT("Buccaneer.EnableEvents"),
	true,
	TEXT("Enables the collection of semantic events (default: true)"),
	ECVF_Default);

TAutoConsoleVariable<FString> UBuccaneerSettings::CVarMetadata(
	TEXT("Buccaneer.Metadata"),
	TEXT(""),
	TEXT(""),
    FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* Var) { Delegates()->OnMetadataChanged.Broadcast(Var); }),
	ECVF_Default);

TAutoConsoleVariable<float> UBuccaneerSettings::CVarReportingInterval(
	TEXT("Buccaneer.ReportingInterval"),
	1.0f,
	TEXT("The interval at which to report performance metrics (default: 1.0 seconds)"),
	ECVF_Default);

UBuccaneerSettings::FDelegates* UBuccaneerSettings::DelegateSingleton = nullptr;

UBuccaneerSettings::FDelegates* UBuccaneerSettings::Delegates()
{
	if (DelegateSingleton == nullptr && !IsEngineExitRequested())
	{
		DelegateSingleton = new UBuccaneerSettings::FDelegates();
		return DelegateSingleton;
	}
	return DelegateSingleton;
}

UBuccaneerSettings::~UBuccaneerSettings()
{
	DelegateSingleton = nullptr;
}

TMap<FString, FString> UBuccaneerSettings::GetMetadata()
{
    TMap<FString, FString> MetadataMap;

    FString MetadataString = CVarMetadata.GetValueOnAnyThread();
    if (!MetadataString.IsEmpty())
    {
        TArray<FString> Pairs;
        MetadataString.ParseIntoArray(Pairs, TEXT(";"), true);
        for (FString Pair : Pairs)
        {
            if (Pair.IsEmpty())
            {
                continue;
            }
        
            FString Key, Value;
            Pair.Split(TEXT(":"), &Key, &Value);
            if(!Key.IsEmpty() && Value.IsEmpty())
            {
                MetadataMap.Add(Key, Value);
            }
        }
    }

    return MetadataMap;
}

FName UBuccaneerSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

#if WITH_EDITOR
FText UBuccaneerSettings::GetSectionText() const
{
	return NSLOCTEXT("BuccaneerPlugin", "BuccaneerSettingsSection", "Buccaneer");
}

void UBuccaneerSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FString PropertyName = PropertyChangedEvent.Property->GetNameCPP();

	FString CVarName;
	if (CVarName = Util::FindCVarFromProperty(GetCmdArg, PropertyName); !CVarName.IsEmpty())
	{
        if (PropertyName == "Metadata")
		{
			Util::SetMetadataCVarFromProperty(this, PropertyChangedEvent.Property);
		}
        else
        {
            SetCVarFromProperty(CVarName, PropertyChangedEvent.Property);
        }
	}
}
#endif

void UBuccaneerSettings::SetCVarAndPropertyFromValue(const FString& CVarName, FProperty* Property, const FString& Value)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (!CVar)
	{
		UE_LOGFMT(LogBuccaneerCommon, Warning, "Failed to find CVar: {0}", CVarName);
		return;
	}

	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property); ByteProperty != NULL && ByteProperty->Enum != NULL)
	{
		CVar->Set(FCString::Atoi(*Value), ECVF_SetByCommandline);
		ByteProperty->SetPropertyValue_InContainer(this, FCString::Atoi(*Value));
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atoi(*Value));
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

			UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), EnumProperty->GetEnum()->GetNameStringByIndex(EnumIndex));
		}
		else
		{
			UE_LOGFMT(LogBuccaneerCommon, Warning, "{0} is not a valid enum value for {1}", Value, EnumProperty->GetEnum()->CppType);
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
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), bValue);
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		CVar->Set(FCString::Atoi(*Value), ECVF_SetByCommandline);
		IntProperty->SetPropertyValue_InContainer(this, FCString::Atoi(*Value));
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atoi(*Value));
	}
	else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		CVar->Set(FCString::Atof(*Value), ECVF_SetByCommandline);
		FloatProperty->SetPropertyValue_InContainer(this, FCString::Atof(*Value));
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [{2}] from command line", CVarName, Property->GetNameCPP(), FCString::Atof(*Value));
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		CVar->Set(*Value, ECVF_SetByCommandline);
		StringProperty->SetPropertyValue_InContainer(this, *Value);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		CVar->Set(*Value, ECVF_SetByCommandline);
		NameProperty->SetPropertyValue_InContainer(this, FName(*Value));
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		// TODO (william.belcher): Only FString array properties are currently supported
		CVar->Set(*Value, ECVF_SetByCommandline);

		TArray<FString> StringArray;
		Value.ParseIntoArray(StringArray, TEXT(","), true);

		TArray<FString>& Array = *ArrayProperty->ContainerPtrToValuePtr<TArray<FString>>(this);
		Array = StringArray;

		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] and Property [{1}] to [\"{2}\"] from command line", CVarName, Property->GetNameCPP(), Value);
	}
}

void UBuccaneerSettings::SetCVarFromProperty(const FString& CVarName, FProperty* Property)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (!CVar)
	{
		UE_LOGFMT(LogBuccaneerCommon, Warning, "Failed to find CVar: {0}", CVarName);
		return;
	}

	if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property); ByteProperty != NULL && ByteProperty->Enum != NULL)
	{
		CVar->Set(ByteProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, ByteProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		void* PropertyAddress = EnumProperty->ContainerPtrToValuePtr<void>(this);
		int64 CurrentValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(PropertyAddress);
		CVar->Set(*EnumProperty->GetEnum()->GetNameStringByValue(CurrentValue), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, EnumProperty->GetEnum()->GetNameStringByValue(CurrentValue), Property->GetNameCPP());
	}
	else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		CVar->Set(BoolProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, BoolProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
	{
		CVar->Set(IntProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, IntProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
	{
		CVar->Set(FloatProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [{1}] from Property [{2}]", CVarName, FloatProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
	{
		CVar->Set(*StringProperty->GetPropertyValue_InContainer(this), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, StringProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		CVar->Set(*NameProperty->GetPropertyValue_InContainer(this).ToString(), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, NameProperty->GetPropertyValue_InContainer(this), Property->GetNameCPP());
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		// TODO (william.belcher): Only FString array properties are currently supported
		TArray<FString> Array = *ArrayProperty->ContainerPtrToValuePtr<TArray<FString>>(this);
		CVar->Set(*FString::Join(Array, TEXT(",")), ECVF_SetByProjectSetting);
		UE_LOGFMT(LogBuccaneerCommon, Log, "Setting CVar [{0}] to [\"{1}\"] from Property [{2}]", CVarName, FString::Join(Array, TEXT(",")), Property->GetNameCPP());
	}
}

void UBuccaneerSettings::InitializeCVarsFromProperties()
{
	UE_LOGFMT(LogBuccaneerCommon, Log, "Initializing CVars from ini");
	for (FProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
	{
		if (!Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

        // Handle the majority of commandline argument
		if (Property->GetNameCPP() == "Metadata")
		{
            Util::SetMetadataCVarFromProperty(this, Property);
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

void UBuccaneerSettings::ValidateCommandLineArgs()
{
	FString CommandLine = FCommandLine::Get();

	TArray<FString> CommandArray;
	CommandLine.ParseIntoArray(CommandArray, TEXT(" "), true);

	for (FString Command : CommandArray)
	{
		Command.RemoveFromStart(TEXT("-"));
		if (!Command.StartsWith("Buccaneer"))
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
			for (const TPair<FString, FString>& Pair : GetLegacyCmdArg)
			{
				FString ValidCommandLineArg = Util::ConsoleVariableToCommandArgParam(Pair.Key);
				if (CurrentCommandLineArg == ValidCommandLineArg)
				{
					bValidArg = true;
					break;
				}
			}
		}

		if (!bValidArg)
		{
			UE_LOGFMT(LogBuccaneerCommon, Warning, "Unknown Buccaneer command line arg: {0}", CurrentCommandLineArg);
		}
	}
}

void UBuccaneerSettings::ParseCommandlineArgs()
{
	UE_LOGFMT(LogBuccaneerCommon, Verbose, "Updating CVars and properties with command line args");
	for (const TPair<FString, FString>& Pair : GetCmdArg)
	{
		FString CVarString = Pair.Key;
		FString PropertyName = Pair.Value;

		FProperty* Property = GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property || !Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

        if (PropertyName == "Metadata")
        {
            FString ConsoleString;
            if (FParse::Value(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgValue(CVarString), ConsoleString))
            {
                Util::SetMetadataCVarAndPropertyFromValue(this, Property, ConsoleString);
            }
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

void UBuccaneerSettings::ParseLegacyCommandlineArgs()
{
	FString BuccaneerIP;
	FString BuccaneerPort;

	for (const TPair<FString, FString>& Pair : GetLegacyCmdArg)
	{
		FString LegacyCVarString = Pair.Key;
		FString PropertyName = Pair.Value;

		FProperty* Property = GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Property || !Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		FString NewCVarString;
		if (FString CmdArgCVar = Util::FindCVarFromProperty(GetCmdArg, PropertyName); !CmdArgCVar.IsEmpty())
		{
			NewCVarString = CmdArgCVar;
		}
		else
		{
			continue;
		}

		if (LegacyCVarString == "Buccaneer.IP" || LegacyCVarString == "Buccaneer.Port")
		{
			if (LegacyCVarString == "Buccaneer.IP")
			{
				FParse::Value(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgValue(LegacyCVarString), BuccaneerIP);
			}
			else if (LegacyCVarString == "Buccaneer.Port")
			{
				FParse::Value(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgValue(LegacyCVarString), BuccaneerPort);
			}

			if (!BuccaneerIP.IsEmpty() && !BuccaneerPort.IsEmpty())
			{
				FString LegacyUrl = TEXT("http://") + BuccaneerIP + TEXT(":") + BuccaneerPort;
				SetCVarAndPropertyFromValue(NewCVarString, Property, LegacyUrl);
				UE_LOGFMT(LogBuccaneerCommon, Warning, "BuccaneerIP and BuccaneerPort are legacy settings converted to -BuccaneerURL={0}", CVarURL.GetValueOnAnyThread());
			}

			continue;
		}

		FString ConsoleString;
		if (FParse::Value(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgValue(LegacyCVarString), ConsoleString))
		{
			SetCVarAndPropertyFromValue(NewCVarString, Property, ConsoleString);
		}
		else if (FParse::Param(FCommandLine::Get(), *Util::ConsoleVariableToCommandArgParam(LegacyCVarString)))
		{
			SetCVarAndPropertyFromValue(NewCVarString, Property, TEXT("true"));
		}
		else
		{
			continue;
		}

		UE_LOGFMT(LogBuccaneerCommon, Warning, "{0} is a legacy setting and has been converted to {1}", Util::ConsoleVariableToCommandArgParam(LegacyCVarString), Util::ConsoleVariableToCommandArgParam(NewCVarString));
	}

	// End legacy buccaneer command line args
}

void UBuccaneerSettings::PostInitProperties()
{
	Super::PostInitProperties();

	UE_LOGFMT(LogBuccaneerCommon, Log, "Initialising Buccaneer settings.");

	// Set all the CVars to reflect the state of the ini
	InitializeCVarsFromProperties();

	// Validate command line args to log if they're invalid
	ValidateCommandLineArgs();

	// Update CVars and properties based on command line args
	ParseCommandlineArgs();

    // Handle parsing of legacy command line args (such as -PixelStreamingIP) after .ini and new commandline args.
	ParseLegacyCommandlineArgs();
}