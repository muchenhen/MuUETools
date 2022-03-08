#pragma once

class FMultiShaderCooker
{
public:
	FMultiShaderCooker();
	~FMultiShaderCooker();
	// 文件保存路径
	FString StorageCookedDir = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()), TEXT("Cooked"));;
	// metadata存储路径
	FString StorageMetadataDir = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()), TEXT("MultiShaderLib"));;
	// 是否清除已存在的文件夹
	bool bCleanExists = true;
	// 打包平台目标
	TArray<FString> GPlatformNames = {"WindowsNoEditor"};
	// 平台实例
	TArray<ITargetPlatform*> GTargetPlatforms;
	// 文件名
	FString ShaderLibraryName;
	// Packages
	TArray<UPackage*> AssetsPackage;
	// AssetData
	TArray<FAssetData> GAssetData;
	
	bool bIsNative = false;

	// 创建shader library
	void CreateShaderCodeLibrary();

protected:
	void Init();
	bool CookAssets();
	void ShutDown();
};
