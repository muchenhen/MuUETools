// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "MuStruct.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MuBlueprintLibraryBPLibrary.generated.h"

/* 

*/
UCLASS()
class UMuBlueprintLibraryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	// MaterialInstance
public:
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void GetFxInvolvedDynamicParameter(const UMaterialInstance* MaterialInstance, TMap<FName, FMTIDynamicParamFloatValueSet>& UseDynamicParameters);

	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void GetMaterialInstanceParametersEditorDisplayed(UMaterialInstance* MaterialInstance, TArray<FString>& ParameterName);

	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void ClearMaterialInstanceNonexistentParameters(UMaterialInstance* MaterialInstance, EMaterialInstanceParameterType MIParameterType);

};
