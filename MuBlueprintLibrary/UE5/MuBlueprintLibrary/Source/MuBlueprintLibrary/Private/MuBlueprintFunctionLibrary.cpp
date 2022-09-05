// Copyright Epic Games, Inc. All Rights Reserved.

#include "MuBlueprintFunctionLibrary.h"
#include "MuBlueprintLibrary.h"
#include "NiagaraEditorModule.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraRibbonRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraSystem.h"
#include "MaterialEditor/DEditorParameterValue.h"
#include "MaterialEditor/MaterialEditorInstanceConstant.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/Parameter/ParticleModuleParameterDynamic.h"

UMuBlueprintFunctionLibrary::UMuBlueprintFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMuBlueprintFunctionLibrary::GetFxInvolvedDynamicParameter(const UMaterialInstance* MaterialInstance, TMap<FName, FMTIDynamicParamFloatValueSet>& UseDynamicParameters)
{
	UseDynamicParameters.Empty();
	if (!MaterialInstance)
	{
		return;
	}
	const UMaterial* Material = MaterialInstance->GetMaterial();
	if (Material == nullptr)
	{
		return;
	}
	const TArray<FName>& ParamNames = Material->GetCachedExpressionData().DynamicParameterNames;
	if (ParamNames.Num() == 0)
	{
		return;
	}
	const FName PackageName(MaterialInstance->GetPackage()->GetName());


	[[maybe_unused]] FNiagaraEditorModule& NiagaraEditorModule = FModuleManager::LoadModuleChecked<FNiagaraEditorModule>("NiagaraEditor");
	TMap<FString, TMap<FName, uint32>> SwitchNodeMap;


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FName> ReferencerPackages;
	AssetRegistry.GetReferencers(PackageName, ReferencerPackages, UE::AssetRegistry::EDependencyCategory::Package, UE::AssetRegistry::EDependencyQuery::Hard);

	TArray<UParticleEmitter*> PSEmitters;
	TArray<UNiagaraEmitter*> NSEmitters;
	for (const FName& ReferencerPackage : ReferencerPackages)
	{
		TArray<FAssetData> Assets;
		AssetRegistry.GetAssetsByPackageName(ReferencerPackage, Assets);

		for (const FAssetData& Asset : Assets)
		{
			if (Asset.GetClass()->IsChildOf<UParticleSystem>())
			{
				if (UParticleSystem* PS = Cast<UParticleSystem>(Asset.GetAsset()))
				{
					PSEmitters.Append(PS->Emitters);
				}
			}
			else if (Asset.GetClass()->IsChildOf<UNiagaraSystem>())
			{
				if (UNiagaraSystem* NS = Cast<UNiagaraSystem>(Asset.GetAsset()))
				{
					const TArray<FNiagaraEmitterHandle>& EmitterHandles = NS->GetEmitterHandles();
					for (int i = 0; i < EmitterHandles.Num(); ++i)
					{
						NSEmitters.Add(EmitterHandles[i].GetInstance());
					}
				}
			}
		}
	}

	const FString PathName = MaterialInstance->GetPathName();


	for (int i = 0; i < PSEmitters.Num(); ++i)
	{
		const UParticleEmitter* Emitter = PSEmitters[i];
		const UParticleLODLevel* LODLevel = Emitter->LODLevels.Num() > 0 ? Emitter->LODLevels[0] : nullptr;
		if (LODLevel && LODLevel->RequiredModule && LODLevel->RequiredModule->Material && LODLevel->RequiredModule->Material->GetPathName() == PathName)
		{
			for (int j = 0; j < LODLevel->Modules.Num(); ++j)
			{
				if (const UParticleModuleParameterDynamic* Module = Cast<const UParticleModuleParameterDynamic>(LODLevel->Modules[j]))
				{
					for (int k = 0; k < Module->DynamicParams.Num(); ++k)
					{
						FEmitterDynamicParameter DynamicParameter = Module->DynamicParams[k];
						if (k < 4)
						{
							if (Cast<UDistributionFloatConstant>(DynamicParameter.ParamValue.Distribution))
							{
								UseDynamicParameters.FindOrAdd(ParamNames[k]).Values.Add(DynamicParameter.ParamValue.GetValue());
								UseDynamicParameters.FindOrAdd(ParamNames[k]).ParameterIndex = k;
							}
							else
							{
								UseDynamicParameters.FindOrAdd(ParamNames[k]).ParameterIndex = k;
							}
						}
					}
				}
			}
		}
	}

	TArray<FString> DynamicParameters;

	for (int i = 0; i < NSEmitters.Num(); ++i)
	{
		bool bOk = false;
		const TArray<UNiagaraRendererProperties*>& Renderers = NSEmitters[i]->GetRenderers();
		for (int j = 0; j < Renderers.Num(); ++j)
		{
			UMaterialInterface* RendererMaterial = nullptr;
			FNiagaraVariableAttributeBinding* Binding = nullptr;
			if (UNiagaraMeshRendererProperties* MeshRenderer = Cast<UNiagaraMeshRendererProperties>(Renderers[j]))
			{
				if (MeshRenderer->DynamicMaterialBinding.DoesBindingExistOnSource())
				{
					TArray<FNiagaraMeshMaterialOverride>& RendererMaterials = MeshRenderer->OverrideMaterials;
					for (auto& AMaterial : RendererMaterials)
					{
						UMaterialInterface* RendererMaterialInterface = AMaterial.ExplicitMat;
						if (RendererMaterialInterface && RendererMaterialInterface->GetPathName() == PathName)
						{
							bOk = true;
							break;
						}
					}
				}
			}
			if (UNiagaraRibbonRendererProperties* RibbonRenderer = Cast<UNiagaraRibbonRendererProperties>(Renderers[j]))
			{
				RendererMaterial = RibbonRenderer->Material;
				Binding = &RibbonRenderer->DynamicMaterialBinding;
			}
			else if (UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderers[j]))
			{
				RendererMaterial = SpriteRenderer->Material;
				Binding = &SpriteRenderer->DynamicMaterialBinding;
			}

			if (RendererMaterial && RendererMaterial->GetPathName() == PathName && Binding && Binding->DoesBindingExistOnSource())
			{
				bOk = true;
				break;
			}
		}
		if (bOk)
		{
			for (int j = 0; j < ParamNames.Num(); ++j)
			{
				UseDynamicParameters.FindOrAdd(FName(ParamNames[j])).ParameterIndex = j;
			}
			break;
		}
	}
}

void UMuBlueprintFunctionLibrary::GetMaterialInstanceParametersEditorDisplayed(UMaterialInstance* MaterialInstance, TArray<FString>& ParameterName)
{
	UMaterialEditorInstanceConstant* MaterialEditorInstance = NewObject<UMaterialEditorInstanceConstant>(MaterialInstance->GetPackage(), NAME_None, RF_Transactional);
	UMaterialInstanceConstant* InstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInstance);
	MaterialEditorInstance->SetSourceInstance(InstanceConstant);
	for (int32 GroupIdx = 0; GroupIdx < MaterialEditorInstance->ParameterGroups.Num(); ++GroupIdx)
	{
		FEditorParameterGroup& ParameterGroup = MaterialEditorInstance->ParameterGroups[GroupIdx];
		for (int32 ParamIdx = 0; ParamIdx < ParameterGroup.Parameters.Num(); ++ParamIdx)
		{
			const UDEditorParameterValue* Parameter = ParameterGroup.Parameters[ParamIdx];
			FString Name = Parameter->ParameterInfo.Name.ToString();
			if (MaterialEditorInstance->VisibleExpressions.Contains(Parameter->ParameterInfo) ? true : false)
			{
				ParameterName.AddUnique(Name);
			}
		}
	}
}

void UMuBlueprintFunctionLibrary::ClearMaterialInstanceNonexistentParameters(UMaterialInstance* MaterialInstance, EMaterialInstanceParameterType MIParameterType)
{
	if (!IsValid(MaterialInstance))
	{
		return;
	}
	const UMaterial* Material = MaterialInstance->GetMaterial();
	if (!IsValid(Material))
	{
		return;
	}
	TArray<FMaterialParameterInfo> MaterialParameterInfos;
	TArray<FName> MaterialParameterNames;
	TArray<FGuid> MaterialGUIDs;

	auto FillNames = [](TArray<FName>& Names, const TArray<FMaterialParameterInfo>& Infos)
	{
		for (auto& Info : Infos)
		{
			Names.Add(Info.Name);
		}
	};

	auto EmptyArrays = [&MaterialParameterInfos, &MaterialParameterNames, &MaterialGUIDs]()
	{
		MaterialParameterInfos.Empty();
		MaterialParameterNames.Empty();
		MaterialGUIDs.Empty();
	};

	bool bDirty = false;

	if (MIParameterType == EMaterialInstanceParameterType::Scalar)
	{
		Material->GetAllScalarParameterInfo(MaterialParameterInfos, MaterialGUIDs);
		FillNames(MaterialParameterNames, MaterialParameterInfos);
		for (int i = (MaterialInstance->ScalarParameterValues.Num() - 1); i >= 0; i--)
		{
			if (! MaterialParameterNames.Contains(MaterialInstance->ScalarParameterValues[i].ParameterInfo.Name))
			{
				MaterialInstance->ScalarParameterValues.RemoveAt(i);
				bDirty = true;
			}
		}
		EmptyArrays();
	}
	else if (MIParameterType == EMaterialInstanceParameterType::Texture)
	{
		Material->GetAllTextureParameterInfo(MaterialParameterInfos, MaterialGUIDs);
		FillNames(MaterialParameterNames, MaterialParameterInfos);
		for (int i = (MaterialInstance->TextureParameterValues.Num() - 1); i >= 0; i--)
		{
			if (! MaterialParameterNames.Contains(MaterialInstance->TextureParameterValues[i].ParameterInfo.Name))
			{
				MaterialInstance->TextureParameterValues.RemoveAt(i);
				bDirty = true;
			}
		}
		EmptyArrays();
	}
	else if (MIParameterType == EMaterialInstanceParameterType::Font)
	{
		Material->GetAllFontParameterInfo(MaterialParameterInfos, MaterialGUIDs);
		FillNames(MaterialParameterNames, MaterialParameterInfos);
		for (int i = (MaterialInstance->FontParameterValues.Num() - 1); i >= 0; i--)
		{
			if (! MaterialParameterNames.Contains(MaterialInstance->FontParameterValues[i].ParameterInfo.Name))
			{
				MaterialInstance->FontParameterValues.RemoveAt(i);
				bDirty = true;
			}
		}
		EmptyArrays();
	}
	else if (MIParameterType == EMaterialInstanceParameterType::Vector)
	{
		Material->GetAllVectorParameterInfo(MaterialParameterInfos, MaterialGUIDs);
		FillNames(MaterialParameterNames, MaterialParameterInfos);
		for (int i = (MaterialInstance->VectorParameterValues.Num() - 1); i >= 0; i--)
		{
			if (! MaterialParameterNames.Contains(MaterialInstance->VectorParameterValues[i].ParameterInfo.Name))
			{
				MaterialInstance->VectorParameterValues.RemoveAt(i);
				bDirty = true;
			}
		}
		EmptyArrays();
	}
	else if (MIParameterType == EMaterialInstanceParameterType::RuntimeVirtualTexture)
	{
		Material->GetAllRuntimeVirtualTextureParameterInfo(MaterialParameterInfos, MaterialGUIDs);
		FillNames(MaterialParameterNames, MaterialParameterInfos);
		for (int i = (MaterialInstance->RuntimeVirtualTextureParameterValues.Num() - 1); i >= 0; i--)
		{
			if (! MaterialParameterNames.Contains(MaterialInstance->RuntimeVirtualTextureParameterValues[i].ParameterInfo.Name))
			{
				MaterialInstance->RuntimeVirtualTextureParameterValues.RemoveAt(i);
				bDirty = true;
			}
		}
		EmptyArrays();
	}

	if (bDirty)
	{
		MaterialInstance->MarkPackageDirty();
	}
}
