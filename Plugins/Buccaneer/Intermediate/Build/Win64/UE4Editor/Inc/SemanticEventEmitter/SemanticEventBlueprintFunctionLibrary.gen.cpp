// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SemanticEventEmitter/Public/SemanticEventBlueprintFunctionLibrary.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
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
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Level;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_Event;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Level = { "Level", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms, Level), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Event = { "Event", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms, Event), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Level,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::NewProp_Event,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams[] = {
		{ "Category", "Buccaneer" },
		{ "ModuleRelativePath", "Public/SemanticEventBlueprintFunctionLibrary.h" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary, nullptr, "EmitSemanticEvent", nullptr, nullptr, sizeof(SemanticEventEmitterBlueprintLibrary_eventEmitSemanticEvent_Parms), Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_NoRegister()
	{
		return USemanticEventEmitterBlueprintLibrary::StaticClass();
	}
	struct Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_SemanticEventEmitter,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USemanticEventEmitterBlueprintLibrary_EmitSemanticEvent, "EmitSemanticEvent" }, // 1715515983
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "SemanticEventBlueprintFunctionLibrary.h" },
		{ "ModuleRelativePath", "Public/SemanticEventBlueprintFunctionLibrary.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USemanticEventEmitterBlueprintLibrary>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::ClassParams = {
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
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USemanticEventEmitterBlueprintLibrary, 1147214189);
	template<> SEMANTICEVENTEMITTER_API UClass* StaticClass<USemanticEventEmitterBlueprintLibrary>()
	{
		return USemanticEventEmitterBlueprintLibrary::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USemanticEventEmitterBlueprintLibrary(Z_Construct_UClass_USemanticEventEmitterBlueprintLibrary, &USemanticEventEmitterBlueprintLibrary::StaticClass, TEXT("/Script/SemanticEventEmitter"), TEXT("USemanticEventEmitterBlueprintLibrary"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USemanticEventEmitterBlueprintLibrary);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
