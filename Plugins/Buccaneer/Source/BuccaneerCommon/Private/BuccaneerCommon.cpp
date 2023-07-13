// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuccaneerCommon.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY(BuccaneerCommon);

FBuccaneerCommonModule *FBuccaneerCommonModule::BuccaneerCommonModule = nullptr;

void FBuccaneerCommonModule::StartupModule()
{
    CVarBuccaneerEnableStats = IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("Buccaneer.EnableStats"),
        true,
        TEXT("Disables the collection of and logging of performance metrics"),
        ECVF_Default);

    CVarBuccaneerEnableEvents = IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("Buccaneer.EnableEvents"),
        true,
        TEXT("Disables the collection and logging of semantic events"),
        ECVF_Default);

    Setup();
}

void FBuccaneerCommonModule::ShutdownModule()
{
}

void FBuccaneerCommonModule::Setup()
{
    ParseCommandLineOption(TEXT("BuccaneerEnableStats"), CVarBuccaneerEnableStats);
    ParseCommandLineOption(TEXT("BuccaneerEnableEvents"), CVarBuccaneerEnableEvents);

    if (!FParse::Value(FCommandLine::Get(), TEXT("BuccaneerURL="), BuccaneerURL))
    {
        FString BuccaneerIP;
        uint16 BuccaneerPort;
        if (FParse::Value(FCommandLine::Get(), TEXT("BuccaneerIP="), BuccaneerIP) && FParse::Value(FCommandLine::Get(), TEXT("BuccaneerPort="), BuccaneerPort))
        {
            // build the proper url.
            BuccaneerURL = FString::Printf(TEXT("http://%s:%d"), *BuccaneerIP, BuccaneerPort);
        }
    }

    if (BuccaneerURL.IsEmpty())
    {
        UE_LOG(BuccaneerCommon, Warning, TEXT("Buccanner events and stats disabled, provide `BuccaneerURL` cmd-args to enable it"));
        CVarBuccaneerEnableEvents->Set(false, ECVF_SetByCommandline);
        CVarBuccaneerEnableStats->Set(false, ECVF_SetByCommandline);
        return;
    }

    // Try and parse an instance ID
    if (!FParse::Value(FCommandLine::Get(), TEXT("BuccaneerID="), InstanceID))
    {   
        // Try and parse a pixel streaming ID for users who don't want to pollute their command line by specifying two IDs
        if (!FParse::Value(FCommandLine::Get(), TEXT("PixelStreamingID="), InstanceID))
        {
            // Generate an instance ID if one isn't provided
            InstanceID = FGuid::NewGuid().ToString();
        }
    }

    // Additional Metadata
    MetadataJson = MakeShareable(new FJsonObject());
    FString CmdLineMetadata;
    if (FParse::Value(FCommandLine::Get(), TEXT("BuccaneerMetadata="), CmdLineMetadata))
    {
        UE_LOG(BuccaneerCommon, Warning, TEXT("%s"), *CmdLineMetadata);
        TArray<FString> ParsedMetadata;
        CmdLineMetadata.ParseIntoArray(ParsedMetadata, TEXT(";"), false);
        for (auto Element : ParsedMetadata)
        {
            FString Key, Value;
            Element.Split(TEXT(":"), &Key, &Value);
            MetadataJson->SetField(*Key, MakeShared<FJsonValueString>((TEXT("%s"), *Value)));
        }
    }

    SetupComplete.Broadcast();
}

FBuccaneerCommonModule *FBuccaneerCommonModule::GetModule()
{
    if (BuccaneerCommonModule)
    {
        return BuccaneerCommonModule;
    }
    FBuccaneerCommonModule *Module = FModuleManager::Get().LoadModulePtr<FBuccaneerCommonModule>("BuccaneerCommon");
    if (Module)
    {
        BuccaneerCommonModule = Module;
    }
    return BuccaneerCommonModule;
}

void FBuccaneerCommonModule::SendStats(TSharedPtr<FJsonObject> JsonObject)
{
    JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *InstanceID)));
    JsonObject->SetField("metadata", MakeShared<FJsonValueObject>(MetadataJson));
    SendHTTP(BuccaneerURL + FString("/stats"), JsonObject);
}

void FBuccaneerCommonModule::SendEvent(TSharedPtr<FJsonObject> JsonObject)
{
    JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *InstanceID)));
    SendHTTP(BuccaneerURL + FString("/event"), JsonObject);
}

void FBuccaneerCommonModule::SendHTTP(FString URL, TSharedPtr<FJsonObject> JsonObject)
{
    FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();

    FString body;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&body);
    if (!ensure(FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter)))
    {
        UE_LOG(BuccaneerCommon, Warning, TEXT("Cannot serialize json object"));
    }

    HttpRequest->SetURL(URL);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(body);
    bool bInFlight = true;
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [&bInFlight](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
        {
            FString ResponseStr, ErrorStr;

            if (bSucceeded && HttpResponse.IsValid())
            {
                ResponseStr = HttpResponse->GetContentAsString();
                if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
                {
                    ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"),
                                               HttpResponse->GetResponseCode(), *ResponseStr);
                }
            }
            else
            {
                ErrorStr = TEXT("No response");
            }

            if (!ErrorStr.IsEmpty())
            {
                UE_LOG(BuccaneerCommon, Warning, TEXT("Push event response: %s"), *ErrorStr);
            }

            bInFlight = false;
        });
    HttpRequest->ProcessRequest();
}

void FBuccaneerCommonModule::ParseCommandLineOption(const TCHAR *Match, IConsoleVariable *CVar)
{
    FString ValueMatch(Match);
    ValueMatch.Append(TEXT("="));
    FString Value;
    if (FParse::Value(FCommandLine::Get(), *ValueMatch, Value))
    {
        if (Value.Equals(FString(TEXT("true")), ESearchCase::IgnoreCase))
        {
            CVar->Set(true, ECVF_SetByCommandline);
        }
        else if (Value.Equals(FString(TEXT("false")), ESearchCase::IgnoreCase))
        {
            CVar->Set(false, ECVF_SetByCommandline);
        }
    }
    else if (FParse::Param(FCommandLine::Get(), Match))
    {
        CVar->Set(true, ECVF_SetByCommandline);
    }
}

IMPLEMENT_MODULE(FBuccaneerCommonModule, BuccaneerCommon)