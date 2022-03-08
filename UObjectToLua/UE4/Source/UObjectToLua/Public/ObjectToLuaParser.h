#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "UObject/UnrealType.h"
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
	static FString UObjectToLuaStringWithIgnore(UObject* Object, FString ignore, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToCSV(UObject* Object, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString UObjectToCSVwithIgnore(UObject* Object, FString ignore, bool DefaultAsEmpty = false);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString GetAimValue(UObject* Object, FString Aim);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* SetAimValue(UClass* classNAme, FString Aim, FString nameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* LuaStringToUObject(UClass* className, FString nameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* LuaStringToSetUObject(UObject* objcet, FString nameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CSVToUObject(UClass* className, FString nameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CSVToSetUObject(UObject* objcet, FString nameAndValue);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static UObject* CreateObject(UClass* className);

	UFUNCTION(BlueprintCallable, Category = "UTL")
	static FString CsvFormat(FString Str);

public:
	static FString ToCompressFloatStr(float F, bool DefaultAsEmpty = false);
	static FString ToObjectStr(UObject* Object, bool DefaultAsEmpty = false);
	static FString ToStructStr(UScriptStruct* Struct, void* Object, bool DefaultAsEmpty = false);

	static FString CheckUproperType(UProperty* Property, void* Object, bool DefaultAsEmpty = false);
	static FString GetPropertyValueLuaStr(UProperty* Property, void* Object, bool DefaultAsEmpty = false);

	//检查Object的Uproperty的类型并把value字符串转换成对应值并赋值
	static void CheckSetType(UProperty* Property, void* Object, FString value);

	//Object的UObjectProperty赋值用，返回一个UObject*
	static UObject* SetObj(UObject* object, FString nameAndValue);
};