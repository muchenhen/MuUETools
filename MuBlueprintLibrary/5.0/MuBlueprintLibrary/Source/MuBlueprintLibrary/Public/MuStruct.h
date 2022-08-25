#pragma once
#include "MuStruct.generated.h"

USTRUCT(BlueprintType)
struct FMTIDynamicParamFloatValueSet
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	int ParameterIndex;
	
	UPROPERTY(BlueprintReadWrite)
	TSet<float> Values;
};

UENUM(BlueprintType)
enum class EMaterialInstanceParameterType : uint8
{
	Scalar,
	Vector,
	Texture,
	Font,
	RuntimeVirtualTexture
};