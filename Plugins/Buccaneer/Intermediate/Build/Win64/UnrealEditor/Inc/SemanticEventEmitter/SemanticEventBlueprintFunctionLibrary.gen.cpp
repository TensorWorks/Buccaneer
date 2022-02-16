// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SemanticEventEmitter/Public/SemanticEventBlueprintFunctionLibrary.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSemanticEventBlueprintFunctionLibrary() {}
// Cross Module References
	SEMANTICEVENTEMITTER_API UClass* Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_NoRegister();
	SEMANTICEVENTEMITTER_API UClass* Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_SemanticEventEmitter();
// End Cross Module References
	DEFINE_FUNCTION(USemanticEventEmitterBlueprintLibrary::execEmitSemanticEvent)
	{
		P_GET_PROPERTY(FStrProperty,Z_Param_Level);
		P_GET_PROPERTY(FStrProperty,Z_Param_Event);
		P_FINISH;
		P_NATIVE_BEGIN;
		USemanticEventEmitterBlueprintLibrary::EmitSemanticEvent(Z_Param_Level,Z_Param_Event);
		P_NATIVE_END;
	}
	void USemanticEventEmitterBlueprintLibrary::StaticRegisterNativesUSemanticEventEmitterBlueprintLibrary()
	{
		UClass* Class = USemanticEventEmitterBlueprintLibrary::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "EmitSemanticEvent", &USemanticEventEmitterBlueprintLibrary::execEmitSemanticEvent },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics
	{
		struct SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms
		{
			FString Level;
			FString Event;
		};
		static const UECodeGen_Private::FStrPropertyParams NewProp_Level;
		static const UECodeGen_Private::FStrPropertyParams NewProp_Event;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Level = { "Level", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms, Level), METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Event = { "Event", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms, Event), METADATA_PARAMS(nullptr, 0) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Level,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Event,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams[] = {
		{ "Category", "Buccaneer" },
		{ "ModuleRelativePath", "Public/SemanticEventBlueprintFunctionLibrary.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary, nullptr, "EmitSemanticEvent", nullptr, nullptr, sizeof(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms), Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(USemanticEventEmitterBlueprintLibrary);
	UClass* Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_NoRegister()
	{
		return USemanticEventEmitterBlueprintLibrary::StaticClass();
	}
	struct Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_SemanticEventEmitter,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent, "EmitSemanticEvent" }, // 1940975039
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SemanticEventBlueprintFunctionLibrary.h" },
		{ "ModuleRelativePath", "Public/SemanticEventBlueprintFunctionLibrary.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USemanticEventEmitterBlueprintLibrary>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::ClassParams = {
		&USemanticEventEmitterBlueprintLibrary::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary()
	{
		if (!Z_Registration_Info_UClass_USemanticEventEmitterBlueprintLibrary.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_USemanticEventEmitterBlueprintLibrary.OuterSingleton, Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_USemanticEventEmitterBlueprintLibrary.OuterSingleton;
	}
	template<> SEMANTICEVENTEMITTER_API UClass* StaticClass<USemanticEventEmitterBlueprintLibrary>()
	{
		return USemanticEventEmitterBlueprintLibrary::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(USemanticEventEmitterBlueprintLibrary);
	struct Z_CompiledInDeferFile_FID_Engine_Plugins_Buccaneer_Source_SemanticEventEmitter_Public_SemanticEventBlueprintFunctionLibrary_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Engine_Plugins_Buccaneer_Source_SemanticEventEmitter_Public_SemanticEventBlueprintFunctionLibrary_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary, USemanticEventEmitterBlueprintLibrary::StaticClass, TEXT("USemanticEventEmitterBlueprintLibrary"), &Z_Registration_Info_UClass_USemanticEventEmitterBlueprintLibrary, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(USemanticEventEmitterBlueprintLibrary), 3549355716U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Engine_Plugins_Buccaneer_Source_SemanticEventEmitter_Public_SemanticEventBlueprintFunctionLibrary_h_666734327(TEXT("/Script/SemanticEventEmitter"),
		Z_CompiledInDeferFile_FID_Engine_Plugins_Buccaneer_Source_SemanticEventEmitter_Public_SemanticEventBlueprintFunctionLibrary_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Engine_Plugins_Buccaneer_Source_SemanticEventEmitter_Public_SemanticEventBlueprintFunctionLibrary_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
