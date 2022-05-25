# UObjectToLua
获取UObject的UPROPERTY信息的静态函数库

#### 更新

##### 2022年5月25日

修改UProperty到FProperty
修改对FProperty的Cast为CastFiled
修改了部分变量名，删除无用变量
修改命名规范

##### 2022-03-08  

迁移仓库到这里，[原仓库](https://github.com/muchenhen/UObjectToLua)

##### 2021-05-29

增加了对结构体的支持,优化了代码

##### 2020-10-21

增加了对TSet和UEnum的支持

#### 注意事项

目前支持基本数据类型，int，float，bool，FString，TArray（支持嵌套），TMap（支持嵌套），自定义类，FVector，FRotator，**无法处理自定义结构体**

对于自定义类如果UPROPERTY存在未进行构造的野指针，会直接返回空字符串。

#### **提供的函数**

###### static FString UObjectToLuaString(UObject* Object);

遍历该Object的所有UPROPERTY，返回一个字符串，包括所有的UPROPERTY的名字和值

###### static FString UObjectToLuaStringWithIgnore(UObject* Object, FString **ignore**);

在遍历所有PROPERTY的基础上，会忽略掉ignore中的UPROPERTY，ignore中是UPROPERTY的名字，用逗号隔开，最后也要有一个逗号

###### static FString UObjectToCSV(UObject* Object);

转换成符合CSV文件规则的字符串

###### static FString UObjectToCSVwithIgnore(UObject* Object, FString ignore);

同上，ignore要求和上上个函数相同

###### static FString GetAimValue(UObject* Object, FString Aim);

仅获取目标UPROPERTY的值，这里Aim只是单个UPROPERTY的名字

###### static UObject* **SetAimValue**(UClass* classNAme, FString Aim, FString nameAndValue);

根据UClass生成一个新的Object，Aim是要设置的目标UPROPERTY的名称，第三个参数是要设置的UPROPERTY的名称和值

nameAndValue的格式如下

```
MyObject={mybool=true,MyChildObject={myChildBool=false,myChildInt=1,},MyTArray={1,2,3,4,},myTMap={'x'=1,'y'=2,'z'=3,},}
```

###### static UObject* LuaStringToUObject(UClass* className, FString nameAndValue);

会创建一个新的Object进行赋值

###### static UObject* **LuaStringToSetUObject**(UObject* objcet, FString nameAndValue);

给一个已有的Object值进行更改

###### static UObject* CSVToUObject(UClass* className, FString nameAndValue);

用符合CSV文件格式要求的字符串生成一个新的Object

###### static UObject* **CSVToSetUObject**(UObject* objcet, FString nameAndValue);

用符合CSV文件格式要求的字符串对一个已有的Object进行赋值

###### static UObject* CreateObject(UClass* className);

通过UClass创建UObject

###### static FString **LuaStringToCSV**(FString LuaString);

将Lua风格的字符串转换为CSV风格