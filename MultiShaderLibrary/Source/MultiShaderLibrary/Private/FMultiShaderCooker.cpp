// ReSharper disable All
#include "FMultiShaderCooker.h"
#include "MultiShaderFunc.h"

using namespace MultiShaderFunc;

FMultiShaderCooker::FMultiShaderCooker()
{
}

FMultiShaderCooker::~FMultiShaderCooker()
{
}

void FMultiShaderCooker::CreateShaderCodeLibrary()
{
	UE_LOG(LogMultiShaderCooker, Display, TEXT("Begin Create Shader Code Library."));
	GAssetData = GetAllAssets();
	if (GAssetData.Num() == 0)
	{
		UE_LOG(LogMultiShaderCooker, Error, TEXT("Plase Choose assets."));
		return;
	}
	void Init();
	bool CookAssets();
	void ShutDown();
}

void FMultiShaderCooker::Init()
{
	// 初始化文件名
	ShaderLibraryName = FString::Printf(TEXT("%s_Shader_%d"), FApp::GetProjectName(), 0);

	if (bCleanExists)
	{
		DeleteDirectory(StorageCookedDir);
		DeleteDirectory(StorageMetadataDir);
	}
	InitCookShaderCollection(ShaderLibraryName, bIsNative, GPlatformNames, GTargetPlatforms);
}

bool FMultiShaderCooker::CookAssets()
{
	bool bSuccessed = false;

    TArray<FSoftObjectPath> Assets;
    for (const auto& AssetData : GAssetData)
    {
        if (AssetData.IsValid())
        {
            Assets.Emplace(AssetData.ObjectPath.ToString());
        }
    }
    for (const auto& PlatformName : GPlatformNames)
    {
        ITargetPlatform* PlatformIns = GetPlatformByName(PlatformName);
        if (PlatformIns)
        {
            GTargetPlatforms.AddUnique(PlatformIns);
        }
    }

    for (int32 index = 0; index < Assets.Num(); ++index)
    {
        UE_LOG(LogMultiShaderCooker, Display, TEXT("%d packages is cooked,Remain %d Total %d"), index, Assets.Num() - index, Assets.Num());
        UPackage* Package = GetPackage(*Assets[index].GetLongPackageName());
        bSuccessed = CookPackage(Package, GTargetPlatforms, StorageCookedDir);
    }
    WaitShaderCompilingComplete();
    UPackage::WaitForAsyncFileWrites();
    return bSuccessed;
}

void FMultiShaderCooker::ShutDown()
{
	WaitCookerFinished();
	ShutDownShaderCollection(ShaderLibraryName, bIsNative, GPlatformNames, GTargetPlatforms, StorageMetadataDir);
}
