// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraDataInterfaceFeatureLevel.h"

#include "NiagaraSystemInstance.h"
#include "NiagaraWorldManager.h"

#define LOCTEXT_NAMESPACE "NiagaraDataInterfaceFeatureLevel"

struct FNiagaraFeatureLevelDIFunctionVersion
{
	enum Type
	{
		InitialVersion = 0,
		AddedPreViewTranslation = 1,

		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

const FName UNiagaraDataInterfaceFeatureLevel::GetIfES31Name(TEXT("GetIfES31ForCPU"));
const FName UNiagaraDataInterfaceFeatureLevel::GetIfSM5Name(TEXT("GetIfSM5ForCPU"));

UNiagaraDataInterfaceFeatureLevel::UNiagaraDataInterfaceFeatureLevel(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Proxy.Reset(new FNiagaraDataIntefaceProxyFeatureLevel());
}

void UNiagaraDataInterfaceFeatureLevel::SetPIData(TEnumAsByte<ERHIFeatureLevel::Type> FeatureLevel, FFeatureLevelDataInterface_InstanceData* PIData)
{
	if (FeatureLevel == ERHIFeatureLevel::ES3_1)
	{
		PIData->ES_31 = 1;
		PIData->SM5 = 0;
	}
	else if(FeatureLevel == ERHIFeatureLevel::SM5)
	{
		PIData->ES_31 = 0;
		PIData->SM5 = 1;
	}
	else
	{
		PIData->ES_31 = 0;
		PIData->SM5 = 0;
	}
}

bool UNiagaraDataInterfaceFeatureLevel::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FFeatureLevelDataInterface_InstanceData* PIData = new (PerInstanceData) FFeatureLevelDataInterface_InstanceData;
	UWorld* World = SystemInstance->GetWorldManager()->GetWorld();
	if (World)
	{
		SetPIData(World->FeatureLevel, PIData);
	}
	else if(GWorld)
	{
		SetPIData(GWorld->FeatureLevel, PIData);
	}
	return true;
}

void UNiagaraDataInterfaceFeatureLevel::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	FNiagaraFunctionSignature Sig;
	
	Sig = FNiagaraFunctionSignature();
	Sig.Name = GetIfES31Name;
#if WITH_EDITORONLY_DATA
	Sig.Description = LOCTEXT("GetIfES31Description", "Get If ES31. If the current feature level is ES31, the integer 1 will be returned");
	Sig.FunctionVersion = 0;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Feature Level")));
	Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("If ES31")), LOCTEXT("GetIfES31Description", "When the current Feature Level is ES31, it returns 1; otherwise, it returns 0"));
	OutFunctions.Add(Sig);

	Sig = FNiagaraFunctionSignature();
	Sig.Name = GetIfSM5Name;
#if WITH_EDITORONLY_DATA
	Sig.Description = LOCTEXT("GetIfSM5Description", "Get If SM5. If the current feature level is SM5, the integer 1 will be returned");
	Sig.FunctionVersion = 0;
#endif
	Sig.bMemberFunction = true;
	Sig.bRequiresContext = false;
	Sig.bSupportsGPU = false;
	Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Feature Level")));
	Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("If SM5")), LOCTEXT("GetIfSM5Description", "When the current Feature Level is SM5, it returns 1; otherwise, it returns 0."));
	OutFunctions.Add(Sig);
}

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceFeatureLevel, GetIfES31ForCPU);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceFeatureLevel, GetIfSM5ForCPU);

void UNiagaraDataInterfaceFeatureLevel::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == GetIfES31Name)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceFeatureLevel, GetIfES31ForCPU)::Bind(this, OutFunc);
	}
	else if (BindingInfo.Name == GetIfSM5Name)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceFeatureLevel, GetIfSM5ForCPU)::Bind(this, OutFunc);
	}
}

#if WITH_EDITORONLY_DATA
bool UNiagaraDataInterfaceFeatureLevel::UpgradeFunctionCall(FNiagaraFunctionSignature& FunctionSignature)
{
	bool bWasChanged = false;

	// Early out for version matching
	if (FunctionSignature.FunctionVersion == FNiagaraFeatureLevelDIFunctionVersion::LatestVersion)
	{
		return bWasChanged;
	}
	
	// Set latest version
	FunctionSignature.FunctionVersion = FNiagaraFeatureLevelDIFunctionVersion::LatestVersion;

	return bWasChanged;
}
#endif

void UNiagaraDataInterfaceFeatureLevel::GetIfES31ForCPU(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FFeatureLevelDataInterface_InstanceData> InstData(Context);

	VectorVM::FExternalFuncRegisterHandler<int32> bES31(Context);

	FFeatureLevelDataInterface_InstanceData* FeatureLevelData = InstData.Get();
	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		*bES31.GetDestAndAdvance() = FeatureLevelData->ES_31;
	}
}

void UNiagaraDataInterfaceFeatureLevel::GetIfSM5ForCPU(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FFeatureLevelDataInterface_InstanceData> InstData(Context);

	VectorVM::FExternalFuncRegisterHandler<int32> bSM5(Context);

	FFeatureLevelDataInterface_InstanceData* FeatureLevelData = InstData.Get();
	for (int32 i = 0; i < Context.GetNumInstances(); ++i)
	{
		*bSM5.GetDestAndAdvance() = FeatureLevelData->SM5;
	}
}


#undef LOCTEXT_NAMESPACE
