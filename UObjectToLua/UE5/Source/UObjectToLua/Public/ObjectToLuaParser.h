#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ObjectToLuaParser.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ObjectToLua, Log, All);

UCLASS()
class UOBJECTTOLUA_API UObjectToLuaParser : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToLuaString(UObject* Object, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToLuaStringWithIgnore(UObject* Object, FString Ignore, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToCSV(UObject* Object, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToCSVWithIgnore(UObject* Object, FString Ignore, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString GetAimValue(UObject* Object, FString Aim);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* SetAimValue(UClass* ClassName, FString Aim, FString NameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* LuaStringToUObject(UClass* ClassName, FString NameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* LuaStringToSetUObject(UObject* Object, FString NameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CSVToUObject(UClass* ClassName, FString NameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CSVToSetUObject(UObject* Object, FString NameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CreateObject(UClass* ClassName);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString CsvFormat(FString Str);

public:
    static bool IsParaphraseChar(const FString& Str, int Index);
	static FString ToCompressFloatStr(float F, bool DefaultAsEmpty = false);
	static FString ToObjectStr(UObject* Object, bool DefaultAsEmpty = false);
    static FString ToStructContentStr(const UStruct* Struct, void* Object, bool DefaultAsEmpty);
    static FString ToStructStr(const UScriptStruct* Struct, void* Object, bool DefaultAsEmpty = false);
    static void GetKeyValueFromLuaStr(const FString& LuaStr, TMap<FString, FString>& KeyValue);

	static FString CheckFPropertyType(FProperty* Property, void* Object, bool DefaultAsEmpty = false);
	static FString GetPropertyValueLuaStr(FProperty* Property, void* Object, bool DefaultAsEmpty = false);
    static void GetArrayFromLuaStr(const FString& LuaStr, TArray<FString>& Arr);

	static void CheckSetType(FProperty* Property, void* Object, FString Value);
    static void CheckSetStruct(const UStruct* Struct, void* Object, FString LuaStr);
};