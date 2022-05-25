#include "ObjectToLuaParser.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"

DEFINE_LOG_CATEGORY(ObjectToLua);

//加载UClass模板
template <typename T>
UClass* loadClassT(const char* cls)
{
    FString path(UTF8_TO_TCHAR(cls));
    int32 index;
    if (!path.FindChar(static_cast<TCHAR>('\''), index))
    {
        // load blueprint widget from cpp, need add '_C' tail
        path = FString::Format(TEXT("Blueprint'{0}_C'"), {path});
    }
    else
    // auto add _C suffix
    {
        // remove last '
        path = path.Left(path.Len() - 1);
        path += TEXT("_C'");
    }

    UClass* classObj = LoadClass<T>(nullptr, *path);
    return classObj;
}

//获取UProperty的模板
template <typename ValueType>
ValueType GetPropertyValue(FProperty* Property, void* Object)
{
    ValueType* SourceAdder = Property->ContainerPtrToValuePtr<ValueType>(Object);
    if (SourceAdder)
    {
        return *SourceAdder;
    }
    return *SourceAdder;
}

//设置UProperty的模板
template <typename ValueType>
bool SetPropertyValue(FProperty* Property, void* Object, ValueType Value)
{
    // Get the property addresses for the object
    ValueType* Adder = Property->ContainerPtrToValuePtr<ValueType>(Object);

    if (Adder == nullptr)
    {
        return false;
    }

    // Set the value on the destination object.
    *Adder = Value;
    return true;
}

FString UObjectToLuaParser::ToCompressFloatStr(float F, bool DefaultAsEmpty)
{
    const float Res = static_cast<int>(F * 100.0f + 0.5f) / 100.0f;
    if (DefaultAsEmpty && FMath::IsNearlyZero(Res))
    {
        return FString();
    }
    else
    {
        return FString::SanitizeFloat(Res);
    }
}

FString UObjectToLuaParser::ToObjectStr(UObject* Object, bool DefaultAsEmpty)
{
    FString TotalALLNames;
    int ElNum = 0;
    //遍历所有被UProperty标记的成员
    for (TFieldIterator<FProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        FProperty* Property = *PropertyIterator;
        FString PropertyName = Property->GetName();

        //判断UProperty类型
        FString KVStr = CheckFPropertyType(Property, Object, DefaultAsEmpty);

        if (!KVStr.IsEmpty())
        {
            if (ElNum > 0)
            {
                TotalALLNames.Append(",");
            }
            TotalALLNames.Append(KVStr);
            ElNum++;
        }
    }

    if (DefaultAsEmpty && TotalALLNames.IsEmpty())
    {
        return FString();
    }
    else
    {
        return FString::Printf(TEXT("{%s}"), *TotalALLNames);
    }
}

FString UObjectToLuaParser::ToStructContentStr(const UStruct* Struct, void* Object, bool DefaultAsEmpty)
{
    FString TotalALLNames;
    int ElNum = 0;
    UField* Field = Struct->Children;
    while (Field)
    {
        // Get Blueprint name
        if (FProperty* Property = Cast<FProperty>(Field))
        {
            FString PropertyName = Property->GetName();

            //判断UProperty类型
            FString KVStr = CheckFPropertyType(Property, Object, DefaultAsEmpty);

            if (!KVStr.IsEmpty())
            {
                if (ElNum > 0)
                {
                    TotalALLNames.Append(",");
                }
                TotalALLNames.Append(KVStr);
                ElNum++;
            }
        }

        // Go to next member
        Field = Field->Next;
    }
    return TotalALLNames;
}

FString UObjectToLuaParser::ToStructStr(const UScriptStruct* Struct, void* Object, bool DefaultAsEmpty)
{
    FString TotalALLNames;
    const UStruct* CurrentStruct = Struct;
    while (CurrentStruct)
    {
        FString StructContent = ToStructContentStr(CurrentStruct, Object, DefaultAsEmpty);
        if (!StructContent.IsEmpty())
        {
            if (!TotalALLNames.IsEmpty())
            {
                TotalALLNames.Append(",");
            }
            TotalALLNames.Append(StructContent);
        }
        CurrentStruct = CurrentStruct->GetSuperStruct();
    }

    if (DefaultAsEmpty && TotalALLNames.IsEmpty())
    {
        return FString();
    }
    else
    {
        return FString::Printf(TEXT("{%s}"), *TotalALLNames);
    }
}

FString UObjectToLuaParser::CheckFPropertyType(FProperty* Property, void* Object, bool DefaultAsEmpty)
{
    const FString& PropertyStr = GetPropertyValueLuaStr(Property, Object, DefaultAsEmpty);
    if (PropertyStr.IsEmpty() && DefaultAsEmpty)
    {
        return FString();
    }
    else
    {
        return FString::Printf(TEXT("%s=%s"), *Property->GetName(), *PropertyStr);
    }
}

FString UObjectToLuaParser::GetPropertyValueLuaStr(FProperty* Property, void* Object, bool DefaultAsEmpty)
{
    if (CastField<FBoolProperty>(Property))
    {
        bool tempBool = GetPropertyValue<bool>(Property, Object);
        if (DefaultAsEmpty && !tempBool)
        {
            return FString();
        }
        else
        {
            return tempBool ? FString("true") : FString("false");
        }
    }
    else if (CastField<FIntProperty>(Property))
    {
        int32 tempInt = GetPropertyValue<int>(Property, Object);

        if (DefaultAsEmpty && tempInt == 0)
        {
            return FString();
        }
        else
        {
            return FString::FromInt(tempInt);
        }
    }
    else if (CastField<FFloatProperty>(Property))
    {
        float tempFloat = GetPropertyValue<float>(Property, Object);
        return ToCompressFloatStr(tempFloat, DefaultAsEmpty);
    }
    else if (CastField<FStrProperty>(Property))
    {
        FString tempString = GetPropertyValue<FString>(Property, Object);
        if (DefaultAsEmpty && tempString.IsEmpty())
        {
            return FString();
        }
        else
        {
            return FString::Printf(TEXT("'%s'"), *tempString);
        }
    }
    else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Object));
        int ArrNum = 0;
        FString ArrayAllNames = "{";
        int NilNum = 0;
        for (int i = 0; i < ArrayHelper.Num(); i++)
        {
            void* ValuePtr = ArrayHelper.GetRawPtr(i);
            FString InnerStr = GetPropertyValueLuaStr(ArrayProperty->Inner, ValuePtr, DefaultAsEmpty);
            if (DefaultAsEmpty && InnerStr.IsEmpty())
            {
                NilNum++;
            }
            else
            {
                if (ArrNum > 0)
                {
                    ArrayAllNames.Append(",");
                }

                ArrNum += NilNum;
                for (int j = 0; j < NilNum; j++)
                {
                    ArrayAllNames.Append("nil,");
                }
                NilNum = 0;

                ArrayAllNames.Append(InnerStr.IsEmpty() ? TEXT("nil") : *InnerStr);
                ArrNum++;
            }
        }
        if (DefaultAsEmpty && ArrNum == 0)
        {
            return FString();
        }
        else
        {
            ArrayAllNames.Append("}");
            return ArrayAllNames;
        }
    }
    else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(Object));
        FString KeyName;
        FString ValueName;
        int MapNum = 0;
        FString MapStr = "{";
        for (int i = 0; i < MapHelper.Num(); i++)
        {
            KeyName = GetPropertyValueLuaStr(MapProperty->KeyProp, MapHelper.GetKeyPtr(i), false);
            if (KeyName.IsEmpty())
            {
                continue;
            }
            ValueName = GetPropertyValueLuaStr(MapProperty->ValueProp, MapHelper.GetValuePtr(i), DefaultAsEmpty);
            if (ValueName.IsEmpty())
            {
                continue;
            }
            if (MapNum > 0)
            {
                MapStr.Append(",");
            }
            MapStr.Append(FString::Printf(TEXT("[%s]=%s"), *KeyName, *ValueName));
            MapNum++;
        }
        if (DefaultAsEmpty && MapNum == 0)
        {
            return FString();
        }
        else
        {
            MapStr.Append("}");
            return MapStr;
        }
    }
    else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        if (StructProperty->Struct == TBaseStructure<FVector>::Get())
        {
            FVector vec = GetPropertyValue<FVector>(Property, Object);
            if (DefaultAsEmpty && vec == FVector::ZeroVector)
            {
                return FString();
            }
            else
            {
                FString VectorAll = "{";
                VectorAll.Append(FString::Printf(TEXT("X=%s,"), *ToCompressFloatStr(vec.X)));
                VectorAll.Append(FString::Printf(TEXT("Y=%s,"), *ToCompressFloatStr(vec.Y)));
                VectorAll.Append(FString::Printf(TEXT("Z=%s}"), *ToCompressFloatStr(vec.Z)));
                return VectorAll;
            }
        }
        else if (StructProperty->Struct == TBaseStructure<FRotator>::Get())
        {
            FRotator rot = GetPropertyValue<FRotator>(Property, Object);
            if (DefaultAsEmpty && rot == FRotator::ZeroRotator)
            {
                return FString();
            }
            else
            {
                FString RotatorAll = "{";
                RotatorAll.Append(FString::Printf(TEXT("Pitch=%s,"), *ToCompressFloatStr(rot.Pitch)));
                RotatorAll.Append(FString::Printf(TEXT("Yaw=%s,"), *ToCompressFloatStr(rot.Yaw)));
                RotatorAll.Append(FString::Printf(TEXT("Roll=%s}"), *ToCompressFloatStr(rot.Roll)));
                return RotatorAll;
            }
        }
        else if (StructProperty->Struct == TBaseStructure<FSoftObjectPath>::Get())
        {
            FSoftObjectPath SoftObjectPath = GetPropertyValue<FSoftObjectPath>(Property, Object);
            if (DefaultAsEmpty && !SoftObjectPath.IsValid())
            {
                return FString();
            }
            else
            {
                return FString::Printf(TEXT("'%s'"), *SoftObjectPath.GetAssetPathString());
            }
        }
        else if (StructProperty->Struct == TBaseStructure<FSoftClassPath>::Get())
        {
            FSoftClassPath SoftClassPath = GetPropertyValue<FSoftClassPath>(Property, Object);
            if (DefaultAsEmpty && !SoftClassPath.IsValid())
            {
                return FString();
            }
            else
            {
                return FString::Printf(TEXT("'%s'"), *SoftClassPath.GetAssetPathString());
            }
        }
        else if (StructProperty->Struct->IsNative())
        {
            return ToStructStr(StructProperty->Struct, Object, DefaultAsEmpty);
        }
        else
        {
            UE_LOG(ObjectToLua, Warning, TEXT("Unrecognized Struct Property: %s"), *Property->GetName());
            if (DefaultAsEmpty)
            {
                return FString();
            }
            else
            {
                return FString("nil");
            }
        }
    }
    else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    {
        UObject* tempUObject = GetPropertyValue<UObject*>(ObjectProperty, Object);

        if (!tempUObject)
        {
            if (DefaultAsEmpty)
            {
                return FString();
            }
            else
            {
                return FString("{}");
            }
        }
        else
        {
            return ToObjectStr(tempUObject, DefaultAsEmpty);
        }
    }
    else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
    {
        FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(Object));
        int SetNum = 0;
        FString SetAllNames = "{";
        for (int i = 0; i < SetHelper.Num(); i++)
        {
            void* ValuePtr = SetHelper.GetElementPtr(i);
            const FString& Key = GetPropertyValueLuaStr(SetProperty->ElementProp, ValuePtr, DefaultAsEmpty);
            if (Key.IsEmpty())
            {
            }
            else
            {
                if (SetNum > 0)
                {
                    SetAllNames.Append(",");
                }
                SetNum++;
                SetAllNames.Append(FString::Printf(TEXT("[%s]=true"), *Key));
            }
        }

        if (DefaultAsEmpty && SetNum == 0)
        {
            return FString();
        }
        else
        {
            SetAllNames.Append("}");
            return SetAllNames;
        }
    }
    else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
    {
        const UEnum* Enum = EnumProperty->GetEnum();
        const FNumericProperty* UnderlyingProp = EnumProperty->GetUnderlyingProperty();
        const int64 EnumValue = UnderlyingProp->GetSignedIntPropertyValue(Property->ContainerPtrToValuePtr<void>(Object));
        if (Enum->IsValidEnumValue(EnumValue))
        {
            if (DefaultAsEmpty && EnumValue == 0)
            {
                return FString();
            }
            else
            {
                return FString::FromInt(EnumValue);
            }
        }
        else
        {
            // For array and map value property, Object is the value of enum
            uint8 Value = *static_cast<uint8*>(Object);
            if (Enum->IsValidEnumValue(Value))
            {
                if (DefaultAsEmpty && Value == 0)
                {
                    return FString();
                }
                else
                {
                    return FString::FromInt(Value);
                }
            }
            else
            {
                UE_LOG(ObjectToLua, Warning, TEXT("Unrecognized Enum Property: %s Value: %d"), *Property->GetName(), EnumValue)
                return FString();
            }
        }
    }

    UE_LOG(ObjectToLua, Warning, TEXT("Unrecognized Property: %s"), *Property->GetName());
    if (DefaultAsEmpty)
    {
        return FString();
    }
    else
    {
        return FString("nil");
    }
}

void UObjectToLuaParser::GetKeyValueFromLuaStr(const FString& LuaStr, TMap<FString, FString>& KeyValue)
{
    FString RestStr = LuaStr;
    //去掉前后最外层的大括号
    if (RestStr[0] == '{')
    {
        RestStr = RestStr.Mid(1, RestStr.Len() - 2);
    }
    FString KeyStr;
    TArray<float> ParamArr;
    while (RestStr.Split("=", &KeyStr, &RestStr))
    {
        if (KeyStr[0] == static_cast<TCHAR>('[') && KeyStr[KeyStr.Len() - 1] == static_cast<TCHAR>(']'))
        {
            KeyStr = KeyStr.Mid(1, KeyStr.Len() - 2);
        }
        switch (RestStr[0])
        {
        case static_cast<TCHAR>('\''): {
            //字符串
            int32 index = 0;
            do
            {
                index = RestStr.Find("'", ESearchCase::CaseSensitive, ESearchDir::FromStart, index + 1);
            } while (index > 0 && IsParaphraseChar(RestStr, index));
            FString ValueStr = RestStr.Left(index + 1);
            KeyValue.Add(KeyStr, ValueStr);
            RestStr = RestStr.Right(RestStr.Len() - index - 1);
            if (!RestStr.IsEmpty())
            {
                if (RestStr[0] != static_cast<TCHAR>(','))
                {
                    UE_LOG(LogTemp, Warning, TEXT("RestStr not correct: %s"), *RestStr);
                }
                RestStr = RestStr.Right(RestStr.Len() - 1);
            }
            break;
        }
        case static_cast<TCHAR>('{'): {
            //数组、字典、Object
            int BraceNum = 1;
            int i;
            for (i = 1; i < RestStr.Len() && BraceNum > 0; i++)
            {
                switch (RestStr[i])
                {
                case static_cast<TCHAR>('\''): {
                    do
                    {
                        i++;
                    } while (!(RestStr[i] == static_cast<TCHAR>('\'') && !IsParaphraseChar(RestStr, i)));
                    break;
                }
                case static_cast<TCHAR>('{'): BraceNum++;
                    break;
                case static_cast<TCHAR>('}'): BraceNum--;
                    break;
                default: break;
                }
            }
            FString ValueStr = RestStr.Left(i);
            KeyValue.Add(KeyStr, ValueStr);
            RestStr = RestStr.Right(RestStr.Len() - i - 1);
            break;
        }
        default: {
            //普通变量
            FString ValueStr;
            if (RestStr.Split(",", &ValueStr, &RestStr))
            {
                KeyValue.Add(KeyStr, ValueStr);
            }
            else
            {
                KeyValue.Add(KeyStr, RestStr);
                RestStr.Empty();
            }
            break;
        }
        }
    }
}

void UObjectToLuaParser::GetArrayFromLuaStr(const FString& LuaStr, TArray<FString>& Arr)
{
    FString RestStr = LuaStr;
    //去掉前后最外层的大括号
    if (RestStr[0] == static_cast<TCHAR>('{'))
    {
        RestStr = RestStr.Mid(1, RestStr.Len() - 2);
    }

    int Left = 0;
    for (int i = 0; i < RestStr.Len(); i++)
    {
        switch (RestStr[i])
        {
        case static_cast<TCHAR>(','): {
            Arr.Add(RestStr.Mid(Left, i - Left));
            Left = i + 1;
            break;
        }
        case static_cast<TCHAR>('{'): {
            int BraceNum = 1;
            while (BraceNum > 0 && i < RestStr.Len())
            {
                i++;
                switch (RestStr[i])
                {
                case static_cast<TCHAR>('\''): {
                    do
                    {
                        i++;
                    } while (!(RestStr[i] == static_cast<TCHAR>('\'') && !IsParaphraseChar(RestStr, i)));
                    break;
                }
                case static_cast<TCHAR>('{'): BraceNum++;
                    break;
                case static_cast<TCHAR>('}'): BraceNum--;
                    break;
                default: break;
                }
            }
            break;
        }
        case static_cast<TCHAR>('\''): {
            do
            {
                i++;
            } while (!(RestStr[i] == static_cast<TCHAR>('\'') && !IsParaphraseChar(RestStr, i)));
            break;
        }
        default: ;
        }
    }
    Arr.Add(RestStr.Right(RestStr.Len() - Left));
}

void UObjectToLuaParser::CheckSetType(FProperty* Property, void* Object, FString LuaStr)
{
    if (CastField<FBoolProperty>(Property))
    {
        bool tempBool = LuaStr.ToBool();
        SetPropertyValue(Property, Object, tempBool);
    }
    else if (CastField<FIntProperty>(Property))
    {
        int32 tempInt = FCString::Atoi(*LuaStr);
        SetPropertyValue(Property, Object, tempInt);
    }
    else if (CastField<FFloatProperty>(Property))
    {
        float tempFloat = FCString::Atof(*LuaStr);
        SetPropertyValue(Property, Object, tempFloat);
    }
    else if (CastField<FStrProperty>(Property))
    {
        SetPropertyValue(Property, Object, LuaStr.Mid(1, LuaStr.Len() - 2));
    }
    else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Object));

        TArray<FString> ValueArr;
        GetArrayFromLuaStr(LuaStr, ValueArr);

        //目标数组元素个数大于0 需要重新设置数组大小
        if (ValueArr.Num() >= 0)
        {
            ArrayHelper.Resize(ValueArr.Num());
        }
        //依次设置数组的每个元素
        for (int i = 0; i < ArrayHelper.Num(); i++)
        {
            if (FObjectProperty* tempObjectProperty = CastField<FObjectProperty>(ArrayProperty->Inner))
            {
                void* ValuePtr = ArrayHelper.GetRawPtr(i);
                UClass* tempClass = tempObjectProperty->PropertyClass;
                UObject* object = NewObject<UObject>(tempClass, tempClass);
                LuaStringToSetUObject(object, ValueArr[i]);

                FProperty* tempProperty = ArrayProperty->Inner;

                *tempProperty->ContainerPtrToValuePtr<UObject*>(ValuePtr) = object;
            }
            else
            {
                void* ValuePtr = ArrayHelper.GetRawPtr(i);
                CheckSetType(ArrayProperty->Inner, ValuePtr, ValueArr[i]);
            }
        }
    }
    else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        TMap<FString, FString> KV;

        GetKeyValueFromLuaStr(LuaStr, KV);

        FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(Object));
        MapHelper.EmptyValues(); //清空原map

        TArray<FString> Keys;
        KV.GetKeys(Keys);
        //设置key和value的值
        for (int i = 0; i < Keys.Num(); i++)
        {
            int index = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
            CheckSetType(MapProperty->KeyProp, MapHelper.GetKeyPtr(index), Keys[i]);
            //当Value为枚举类型时会解析失败，需要修复
            CheckSetType(MapProperty->ValueProp, MapHelper.GetValuePtr(index), KV[Keys[i]]);
        }
        MapHelper.Rehash();
    }
    else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        if (StructProperty->Struct == TBaseStructure<FVector>::Get())
        {
            FString tempFv;
            TMap<FString, float> Fv;

            FString KeyStr;
            FString NumStr;
            FString RestStr = LuaStr.StartsWith("{") ? LuaStr.Mid(1, LuaStr.Len() - 2) : LuaStr;
            while (RestStr.Split(",", &NumStr, &RestStr))
            {
                NumStr.Split("=", &KeyStr, &NumStr);
                Fv.Add(KeyStr, FCString::Atof(*NumStr));
            }
            RestStr.Split("=", &KeyStr, &NumStr);
            Fv.Add(KeyStr, FCString::Atof(*NumStr));
            SetPropertyValue(Property, Object, FVector(Fv["X"], Fv["Y"], Fv["Z"]));
        }
        else if (StructProperty->Struct == TBaseStructure<FRotator>::Get())
        {
            FString tempFr;
            TMap<FString, float> Fv;
            FString KeyStr;
            FString NumStr;
            FString RestStr = LuaStr;
            while (RestStr.Split(",", &NumStr, &RestStr))
            {
                NumStr.Split("=", &KeyStr, &NumStr);
                Fv.Add(KeyStr, FCString::Atof(*NumStr));
            }
            RestStr.Split("=", &KeyStr, &NumStr);
            Fv.Add(KeyStr, FCString::Atof(*NumStr));
            SetPropertyValue(Property, Object, FRotator(Fv["Pitch"], Fv["Yaw"], Fv["Roll"]));
        }
        else if (StructProperty->Struct == TBaseStructure<FSoftObjectPath>::Get())
        {
            LuaStr = LuaStr.Mid(1, LuaStr.Len() - 2);
            SetPropertyValue(Property, Object, FSoftObjectPath(LuaStr));
        }
        else if (StructProperty->Struct == TBaseStructure<FSoftClassPath>::Get())
        {
            LuaStr = LuaStr.Mid(1, LuaStr.Len() - 2);
            SetPropertyValue(Property, Object, FSoftClassPath(LuaStr));
        }
        else if (StructProperty->Struct->IsNative())
        {
            CheckSetStruct(StructProperty->Struct, Object, LuaStr);
        }
    }
    else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
    {
        TMap<FString, FString> KV;

        GetKeyValueFromLuaStr(LuaStr, KV);

        FScriptSetHelper setHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(Object));
        setHelper.EmptyElements();

        TArray<FString> Keys;
        KV.GetKeys(Keys);
        for (int i = 0; i < Keys.Num(); i++)
        {
            setHelper.AddUninitializedValue();
            void* valuePtr = setHelper.GetElementPtr(i);
            CheckSetType(SetProperty->ElementProp, valuePtr, Keys[i]);
        }
    }
    else if (CastField<FEnumProperty>(Property))
    {
        int64 Value = FCString::Atoi(*LuaStr);
        SetPropertyValue(Property, Object, Value);
    }
    else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
    {
        UObject* LocalObject = GetPropertyValue<UObject*>(ObjectProperty, Object);
        if (LocalObject == nullptr)
        {
            LocalObject = NewObject<UObject>(ObjectProperty->PropertyClass, ObjectProperty->PropertyClass);
        }
        LuaStringToSetUObject(LocalObject, LuaStr);
    }
}

void UObjectToLuaParser::CheckSetStruct(const UStruct* Struct, void* Object, FString LuaStr)
{
    TMap<FString, FString> KeyValue;
    GetKeyValueFromLuaStr(LuaStr, KeyValue);

    const UStruct* CurrentStruct = Struct;
    while (CurrentStruct)
    {
        UField* Field = CurrentStruct->Children;
        while (Field)
        {
            // Get Blueprint name
            if (FProperty* Property = Cast<FProperty>(Field))
            {
                FString PropertyName = Property->GetName();
                if (KeyValue.Contains(PropertyName))
                {
                    CheckSetType(Property, Object, KeyValue[PropertyName]);
                }
            }

            // Go to next member
            Field = Field->Next;
        }
        CurrentStruct = CurrentStruct->GetSuperStruct();
    }
}

bool UObjectToLuaParser::IsParaphraseChar(const FString& Str, int Index)
{
    int Count = 0;
    const int i = Index - 1;
    while (i >= 0 && Str[i] == static_cast<TCHAR>('\\'))
    {
        Count++;
    }
    return Count % 2 == 1;
}

FString UObjectToLuaParser::UObjectToLuaString(UObject* Object, bool DefaultAsEmpty)
{
    return FString::Printf(TEXT("return%s"), *ToObjectStr(Object, DefaultAsEmpty));
}

FString UObjectToLuaParser::UObjectToLuaStringWithIgnore(UObject* Object, FString Ignore, bool DefaultAsEmpty)
{
    TArray<FString> ignoreArray;
    FString MatchStr;
    FString RestStr = Ignore;
    while (RestStr.Split(",", &MatchStr, &RestStr))
    {
        ignoreArray.Add(MatchStr);
    }
    ignoreArray.Add(RestStr);

    if (ignoreArray.Num() <= 0)
    {
        return UObjectToLuaParser::UObjectToLuaString(Object, DefaultAsEmpty);
    }

    FString TotalALLNames;
    int ElNum = 0;

    //遍历所有被UProperty标记的成员
    for (TFieldIterator<FProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        FProperty* Property = *PropertyIterator;
        FString PropertyName = Property->GetName();

        //判断需要忽略的属性名称
        if (ignoreArray.Contains(PropertyName))
        {
            continue;
        }

        //判断UProperty类型
        FString KVStr = CheckFPropertyType(Property, Object, DefaultAsEmpty);
        if (!KVStr.IsEmpty())
        {
            if (ElNum > 0)
            {
                TotalALLNames.Append(",");
            }
            TotalALLNames.Append(KVStr);
            ElNum++;
        }
    }
    if (DefaultAsEmpty && TotalALLNames.IsEmpty())
    {
        return FString();
    }
    else
    {
        return FString::Printf(TEXT("return{%s}"), *TotalALLNames);
    }
}

FString UObjectToLuaParser::UObjectToCSV(UObject* Object, bool DefaultAsEmpty)
{
    FString csv = CsvFormat(ToObjectStr(Object, DefaultAsEmpty));
    csv.RemoveAt(0, 1);
    csv.RemoveAt(csv.Len() - 1, 1);
    return csv;
}

FString UObjectToLuaParser::UObjectToCSVWithIgnore(UObject* Object, FString Ignore, bool DefaultAsEmpty)
{
    FString csv = CsvFormat(ToObjectStr(Object, DefaultAsEmpty));
    csv.RemoveAt(0, 1);
    csv.RemoveAt(csv.Len() - 1, 1);
    return csv;
}

FString UObjectToLuaParser::GetAimValue(UObject* Object, FString Aim)
{
    FString TotalALLNames;

    //遍历所有被UProperty标记的成员
    for (TFieldIterator<FProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        FProperty* Property = *PropertyIterator;
        FString PropertyName = Property->GetName();

        if (Aim == PropertyName)
        {
            TotalALLNames = UObjectToLuaParser::CheckFPropertyType(Property, Object);
            break;
        }
    }

    return TotalALLNames;
}

UObject* UObjectToLuaParser::SetAimValue(UClass* ClassName, FString Aim, FString NameAndValue)
{
    if (NameAndValue[0] == '"' && NameAndValue[NameAndValue.Len() - 1] == '"')
    {
        NameAndValue.RemoveAt(0, 1);
        NameAndValue.RemoveAt(NameAndValue.Len() - 1, 1);
    }
    for (int i = 0; i < NameAndValue.Len(); i++)
    {
        if (NameAndValue[i] == '"' && NameAndValue[i + 1] == '"')
        {
            NameAndValue.RemoveAt(i, 1);
            i++;
        }
    }

    UObject* Object = NewObject<UObject>(ClassName, ClassName);

    //如果有return开头的话就去掉
    NameAndValue = NameAndValue.Replace(TEXT("return"), TEXT(""));

    //去掉前后最外层的大括号
    if (NameAndValue[0] == '{')
    {
        NameAndValue.RemoveAt(0, 1);
        NameAndValue.RemoveAt(NameAndValue.Len() - 1, 1);
    }

    FString name;               //临时存放变量名
    FString value;              //临时存放变量值
    TArray<FString> nameArray;  //变量名的数组
    TArray<FString> valueArray; //变量值的数组
    TMap<FString, FString> NaVMap;
    int flags = 0;       //标记等号
    int leftBigPara = 0; //标记{

    //把字符串拆好 放进两个数组里
    for (int i = 0; i < NameAndValue.Len(); i++)
    {
        if (NameAndValue[i] == '=')
        {
            flags++;
            if (flags > 1)
            {
                value += NameAndValue[i];
            }
        }

        if (NameAndValue[i] != '=')
        {
            if (NameAndValue[i] == '}')
            {
                leftBigPara--;
            }

            if (flags) //等号数量大于零
            {
                value += NameAndValue[i];
            }
            else
            {
                name += NameAndValue[i];
            }

            if (leftBigPara == 0)
            {
                if (NameAndValue[i] == '}')
                {
                    i++;
                    //value.RemoveAt(0, 1);
                    nameArray.Add(name);
                    valueArray.Add(value);
                    NaVMap.Add(name, value);
                    name.Empty();
                    value.Empty();
                    leftBigPara = 0;
                    flags = 0;
                    continue;
                }

                if (flags == 1 && NameAndValue[i] == ',')
                {
                    value.RemoveAt(value.Len() - 1, 1);
                    nameArray.Add(name);
                    valueArray.Add(value);
                    NaVMap.Add(name, value);
                    name.Empty();
                    value.Empty();
                    leftBigPara = 0;
                    flags = 0;
                    continue;
                }

                if (flags >= 1 && leftBigPara == 0 && NameAndValue[i] == ',' && NameAndValue[i - 1] == '\'')
                {
                    value.RemoveAt(value.Len() - 1, 1);
                    nameArray.Add(name);
                    valueArray.Add(value);
                    NaVMap.Add(name, value);
                    name.Empty();
                    value.Empty();
                    leftBigPara = 0;
                    flags = 0;
                    continue;
                }
            }
            continue;
        }
        else
        {
            if (NameAndValue[i + 1] == '{')
            {
                leftBigPara++;
                if (NameAndValue[i] != '=')
                {
                    value += NameAndValue[i];
                }
            }
        }
    }

    //开始set
    for (TFieldIterator<FProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        FProperty* Property = *PropertyIterator;
        FString PropertyName = Property->GetName();

        for (int i = 0; i < NaVMap.Num(); i++)
        {
            if (PropertyName == nameArray[i] && nameArray[i] == Aim)
            {
                CheckSetType(Property, Object, valueArray[i]);
                break;
            }
        }
    }

    return Object;
}

UObject* UObjectToLuaParser::LuaStringToUObject(UClass* ClassName, FString NameAndValue)
{
    UObject* Object = NewObject<UObject>(ClassName, ClassName);
    Object = UObjectToLuaParser::LuaStringToSetUObject(Object, NameAndValue);
    return Object;
}

UObject* UObjectToLuaParser::LuaStringToSetUObject(UObject* Object, FString NameAndValue)
{
    //如果有return开头的话就去掉
    const FString ParsingStr = NameAndValue.Replace(TEXT("return"), TEXT(""));

    TMap<FString, FString> NaVMap;
    GetKeyValueFromLuaStr(ParsingStr, NaVMap);

    //开始set
    for (TFieldIterator<FProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        FProperty* Property = *PropertyIterator;
        FString PropertyName = Property->GetName();

        if (NaVMap.Contains(PropertyName))
        {
            CheckSetType(Property, Object, NaVMap[PropertyName]);
        }
    }

    return Object;
}

UObject* UObjectToLuaParser::CSVToUObject(UClass* ClassName, FString NameAndValue)
{
    if (NameAndValue[0] == '"' && NameAndValue[NameAndValue.Len() - 1] == '"')
    {
        NameAndValue.RemoveAt(0, 1);
        NameAndValue.RemoveAt(NameAndValue.Len() - 1, 1);
    }
    for (int i = 0; i < NameAndValue.Len(); i++)
    {
        if (NameAndValue[i] == '"' && NameAndValue[i + 1] == '"')
        {
            NameAndValue.RemoveAt(i, 1);
            i++;
        }
    }
    return UObjectToLuaParser::LuaStringToUObject(ClassName, NameAndValue);
}

UObject* UObjectToLuaParser::CSVToSetUObject(UObject* Object, FString NameAndValue)
{
    if (NameAndValue[0] == '"' && NameAndValue[NameAndValue.Len() - 1] == '"')
    {
        NameAndValue.RemoveAt(0, 1);
        NameAndValue.RemoveAt(NameAndValue.Len() - 1, 1);
    }
    for (int i = 0; i < NameAndValue.Len(); i++)
    {
        if (NameAndValue[i] == '"' && NameAndValue[i + 1] == '"')
        {
            NameAndValue.RemoveAt(i, 1);
            i++;
        }
    }
    return UObjectToLuaParser::LuaStringToSetUObject(Object, NameAndValue);
}

UObject* UObjectToLuaParser::CreateObject(UClass* ClassName)
{
    UObject* Object = NewObject<UObject>(ClassName, ClassName);
    return Object;
}

FString UObjectToLuaParser::CsvFormat(FString Str)
{
    FString csv = Str;
    if (csv.Contains(TEXT(","), ESearchCase::CaseSensitive, ESearchDir::FromStart)
        || csv.Contains(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart)
        || csv.Contains(TEXT("\n"), ESearchCase::CaseSensitive, ESearchDir::FromStart))
    {
        for (int j = 0; j < csv.Len(); j++)
        {
            if (csv[j] == '"')
            {
                csv.InsertAt(j, '"');
                j++;
            }
        }
        csv.InsertAt(0, "\"");
        csv.InsertAt(csv.Len(), "\"");
    }
    return csv;
}
