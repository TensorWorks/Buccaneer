// Copyright TensorWorks Pty Ltd. All Rights Reserved.

#include "BuccaneerCommonModule.h"

#include "BuccaneerSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Logging.h"

#include "HAL/FileManager.h"

void FBuccaneerCommonModule::StartupModule()
{
    if (UBuccaneerSettings::CVarURL.GetValueOnAnyThread().IsEmpty() && !UBuccaneerSettings::CVarEnableJSONOutput.GetValueOnAnyThread())
    {
        UE_LOGFMT(LogBuccaneerCommon, Warning, "Buccanner events and stats disabled, provide `BuccaneerURL` or `BuccaneerEnableJSONOutput` cmd-args to enable it");
        UBuccaneerSettings::CVarEnableStats->Set(false, ECVF_SetByCommandline);
        UBuccaneerSettings::CVarEnableEvents->Set(false, ECVF_SetByCommandline);
        return;
    }

    FString InstanceIDOverride;
    // Try and parse a pixel streaming ID for users who don't want to pollute their command line by specifying two IDs
    if (FParse::Value(FCommandLine::Get(), TEXT("PixelStreamingID="), InstanceIDOverride))
    {
        UBuccaneerSettings::CVarID->Set(*InstanceIDOverride, ECVF_SetByCommandline);
    }

    if (UBuccaneerSettings::FDelegates* Delegates = UBuccaneerSettings::Delegates())
	{
		Delegates->OnMetadataChanged.AddRaw(this, &FBuccaneerCommonModule::FormatMetadata);
	}

    FormatMetadata(nullptr);

    bModuleReady = true;
    ReadyEvent.Broadcast(*this);
}

void FBuccaneerCommonModule::ShutdownModule()
{
}

FBuccaneerCommonModule::FReadyEvent& FBuccaneerCommonModule::OnReady()
{
    return ReadyEvent;
}

bool FBuccaneerCommonModule::IsReady()
{
    return bModuleReady;
}

void FBuccaneerCommonModule::SendStats(TSharedPtr<FJsonObject> JsonObject)
{
    JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *UBuccaneerSettings::CVarID.GetValueOnAnyThread())));
    JsonObject->SetField("metadata", MakeShared<FJsonValueObject>(MetadataJson));

    if (UBuccaneerSettings::CVarEnableJSONOutput.GetValueOnAnyThread())
    {
        SendJSON(TEXT("stats.json"), JsonObject);
    }
    else
    {
        SendHTTP(UBuccaneerSettings::CVarURL.GetValueOnAnyThread() + FString("/stats"), JsonObject);
    }
}

void FBuccaneerCommonModule::SendEvent(TSharedPtr<FJsonObject> JsonObject)
{
    JsonObject->SetField("id", MakeShared<FJsonValueString>((TEXT("%s"), *UBuccaneerSettings::CVarID.GetValueOnAnyThread())));

    if (UBuccaneerSettings::CVarEnableJSONOutput.GetValueOnAnyThread())
    {
        SendJSON(TEXT("events.json"), JsonObject);
    }
    else
    {
        SendHTTP(UBuccaneerSettings::CVarURL.GetValueOnAnyThread() + FString("/event"), JsonObject);
    }
}

void FBuccaneerCommonModule::SendJSON(FString FileName, TSharedPtr<FJsonObject> JsonObject)
{
    FString FilePath = FPaths::Combine(UBuccaneerSettings::CVarJSONOutputDirectory.GetValueOnAnyThread(), FileName);
	
//  This is how we turn the JSON object into a string
	
    FString JsonString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
    if (!ensure(FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter)))
    {
        UE_LOGFMT(LogBuccaneerCommon, Warning, "Cannot serialize json object");
        return;
    }
	
	IFileManager& FileManager = IFileManager::Get();

    // Check if file exists
    bool bFileExists = FileManager.FileExists(*FilePath);

    // Open for read/write (no "truncate")
    TUniquePtr<FArchive> FileAr(FileManager.CreateFileWriter(*FilePath, FILEWRITE_Append ));

    if (!FileAr)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open file for append: %s"), *FilePath);
        return;
    }

     // If file is empty or just an empty array like "[]", start a new array.
    if (FileAr->TotalSize() <= 2)  
    {
        // First time writing OR writing to a corrupted file.
        FileAr->Seek(0);
        FString Start = TEXT("[\n") + JsonString + TEXT("\n]");
        FTCHARToUTF8 Converter(*Start);
        FileAr->Serialize((UTF8CHAR*)Converter.Get(), Converter.Length());
    }
    else
    {
        // Not first time writing: Insert new JSON at correct position. 
        // Seek before the last two characters, assuming they are '\n]'.  
		FileAr->Seek(FileAr->TotalSize() - 2);
        FString Content = TEXT(",\n") + JsonString + TEXT("\n]");
		FTCHARToUTF8 Converter(*Content);
        FileAr->Serialize((UTF8CHAR*)Converter.Get(), Converter.Length());
    }

    FileAr->Close();
}

void FBuccaneerCommonModule::SendHTTP(FString URL, TSharedPtr<FJsonObject> JsonObject)
{
    FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();

    FString Body;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Body);
    if (!ensure(FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter)))
    {
        UE_LOGFMT(LogBuccaneerCommon, Warning, "Cannot serialize json object");
    }

    HttpRequest->SetURL(URL);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetContentAsString(Body);
    HttpRequest->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
    {
        FString ResponseStr, ErrorStr;

        if (bSucceeded && HttpResponse.IsValid())
        {
            ResponseStr = HttpResponse->GetContentAsString();
            if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
            {
                ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"), HttpResponse->GetResponseCode(), *ResponseStr);
            }
        }
        else
        {
            ErrorStr = TEXT("No response");
        }

        if (!ErrorStr.IsEmpty())
        {
            UE_LOGFMT(LogBuccaneerCommon, Warning, "Push event response: {0}", *ErrorStr);
        }
    });

    HttpRequest->ProcessRequest();
}

void FBuccaneerCommonModule::FormatMetadata(IConsoleVariable* Var)
{
    // Additional Metadata
    TMap<FString, FString> MetadataMap = UBuccaneerSettings::GetMetadata();
    for (const TPair<FString, FString>& Pair : MetadataMap)
    {
        if(Pair.Key.IsEmpty() || Pair.Value.IsEmpty())
        {
            continue;
        }
        
        MetadataJson->SetField(*Pair.Key, MakeShared<FJsonValueString>((TEXT("%s"), *Pair.Value)));
    }
}

IMPLEMENT_MODULE(FBuccaneerCommonModule, BuccaneerCommon)
