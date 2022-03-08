#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogMultiShaderCooker, Log, All);

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 26
// 4.27开始有了FShaderLibraryCooker
#define SHADER_COOKER_CLASS FShaderLibraryCooker
#else
#define SHADER_COOKER_CLASS FShaderCodeLibrary
#endif

namespace MultiShaderFunc
{
	// 获取选中的所有Asset
	TArray<FAssetData> GetAllAssets();
	// 获取所有选中的Assets
	TArray<FAssetData> GetSelectedAssetsInBrowserContent();
	// 获取所有选中的文件夹名
	TArray<FString> GetSelectedFolderInBrowserContent();
	// 获取文件夹中的所有Asset
	void GetAssetDataInPaths(const TArray<FString>& Paths, TArray<FAssetData>& OutAssetData);
	// 删除指定文件夹
	void DeleteDirectory(const FString& Dir);
	// 生成文件名
	FString GenerateShaderCodeLibraryName(FString const& Name, bool bIsIterateSharedBuild);


	/*
	 *
	 *
	 */
	// 初始化shader code library收集
	void InitCookShaderCollection(const FString& ShaderLibraryName, const bool& bIsNative, TArray<FString> GPlatformNames, TArray<ITargetPlatform*>GTargetPlatforms);


	// 通过字符串平台名称获取平台实例
	ITargetPlatform* GetPlatformByName(const FString& Name);
	// 通过目标平台实例获取支持的shader格式
	TArray<FName> GetShaderFormatsByTargetPlatform(const ITargetPlatform* TargetPlatform);
	// 获取shader格式描述符
	TArray<SHADER_COOKER_CLASS::FShaderFormatDescriptor> GetShaderFormatsWithStableKeys(const TArray<FName>& ShaderFormats, bool bNeedShaderStableKeys = true, bool bNeedsDeterministicOrder = false);


	/*
	 *
	 *
	 */

	
	
	// 等待cook结束
	void WaitCookerFinished();
	// 等待shader编译完毕
	void WaitShaderCompilingComplete();
	// 通过平台名找到可以用的shader library
	TArray<FString> FindCookedShaderLibByPlatform(const FString& PlatformName, const FString& Directory, bool bRecursive = false);
	// shader code library收集完毕
	void ShutDownShaderCollection(const FString& ShaderLibraryName, const bool& bIsNative, TArray<FString> GPlatformNames, TArray<ITargetPlatform*>GTargetPlatforms,const FString& StorageMetadataDir);
	// 保存shader code library文件
	bool SaveShaderLibrary(const ITargetPlatform* TargetPlatform, FString const& Name, const FString& SaveBaseDir);

	// 判断是否是iOS
	bool IsAppleMetalPlatform(ITargetPlatform* TargetPlatform);

	bool CookPackage(UPackage* Package, TArray<ITargetPlatform*> GTargetPlatforms, FString StorageCookedDir);

	static uint32 GetCookSaveFlag(bool bUnversion, bool bStorageConcurrent, bool CookLinkerDiff);

	static EObjectFlags GetObjectFlagForCooked(UPackage* Package);

	FString ConvertToFullSandboxPath(const FString& FileName, bool bForWrite);

	// 转换PackageName到路径
	FString LongPackageNameToPackagePath(const FString& InLongPackageName);
	// 转换cook文件保存路径
	FString GetAssetCookedSavePath(const FString& BaseDir, const FString& PackageName, const FString& Platform);


	// 获取Package
	UPackage* GetPackage(FName PackageName);
};
