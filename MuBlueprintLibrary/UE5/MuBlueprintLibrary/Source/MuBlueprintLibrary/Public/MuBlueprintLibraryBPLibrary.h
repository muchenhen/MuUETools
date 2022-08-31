// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "MuStruct.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MuBlueprintLibraryBPLibrary.generated.h"

/* 
一些工具函数
*/
UCLASS()
class UMuBlueprintLibraryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	// MaterialInstance
public:
	/**
	 * @brief 获取指定材质实例有被特效等覆盖的动态参数的列表，如果是级联粒子可以获取到对应的值
	 * @param MaterialInstance 材质实例
	 * @param UseDynamicParameters key为参数名， value是该参数的index和值
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void GetFxInvolvedDynamicParameter(const UMaterialInstance* MaterialInstance, TMap<FName, FMTIDynamicParamFloatValueSet>& UseDynamicParameters);

	/**
	 * @brief 获取指定材质实例的一个参数名的列表，该列表和编辑器下打开材质实例编辑器显示的参数名相同，与是否启用和是否override不相同
	 * @param MaterialInstance 材质实例
	 * @param ParameterName 参数名的列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void GetMaterialInstanceParametersEditorDisplayed(UMaterialInstance* MaterialInstance, TArray<FString>& ParameterName);

	/**
	 * @brief 对指定的材质实例进行参数检查，检查指定的参数类型，如果存在该材质实例拥有但是母材质不存在的参数，则会remove掉
	 * @param MaterialInstance 材质实例
	 * @param MIParameterType 材质实例的参数类型
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting | MaterialInstance")
	static void ClearMaterialInstanceNonexistentParameters(UMaterialInstance* MaterialInstance, EMaterialInstanceParameterType MIParameterType);

};
