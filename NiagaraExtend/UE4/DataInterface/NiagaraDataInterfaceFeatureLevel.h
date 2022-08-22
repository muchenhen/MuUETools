// Author：MuChenhen

#pragma once

#include "NiagaraCommon.h"
#include "NiagaraShared.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceFeatureLevel.generated.h"

struct FFeatureLevelDataInterface_InstanceData
{
	int32 ES_31 = 1;
	int32 SM5 = 0;
};

UCLASS(EditInlineNew, meta = (DisplayName = "Feature Level"))
class NIAGARA_API UNiagaraDataInterfaceFeatureLevel : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()
	
public:
	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)override;
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction &OutFunc) override;
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override { return sizeof(FFeatureLevelDataInterface_InstanceData); }
#if WITH_EDITORONLY_DATA
	virtual bool UpgradeFunctionCall(FNiagaraFunctionSignature& FunctionSignature) override;
#endif

	void GetIfES31ForCPU(FVectorVMContext& Context);
	void GetIfSM5ForCPU(FVectorVMContext& Context);
	
	void SetPIData(TEnumAsByte<ERHIFeatureLevel::Type> FeatureLevel, FFeatureLevelDataInterface_InstanceData* PIData);

private:
	static const FName GetIfES31Name;
	static const FName GetIfSM5Name;
};

struct FNiagaraDataIntefaceProxyFeatureLevel : public FNiagaraDataInterfaceProxy
{
	// There's nothing in this proxy.
	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override
	{
		return 0;
	}
};