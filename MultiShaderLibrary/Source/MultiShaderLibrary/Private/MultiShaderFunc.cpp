// ReSharper disable All
#include "MultiShaderFunc.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ShaderCompiler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Chaos/AABB.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetStringLibrary.h"

DEFINE_LOG_CATEGORY(LogMultiShaderCooker);


TArray<FAssetData> MultiShaderFunc::GetAllAssets()
{
	TArray<FAssetData> AssetDataArray = GetSelectedAssetsInBrowserContent();
	const TArray<FString> SelectedFolderArray = GetSelectedFolderInBrowserContent();
	TArray<FAssetData> FolderAssetDataArray;
	// 获取文件夹下的所有assets
	GetAssetDataInPaths(SelectedFolderArray,FolderAssetDataArray);
	AssetDataArray.Append(FolderAssetDataArray);
	return AssetDataArray;
}

TArray<FAssetData> MultiShaderFunc::GetSelectedAssetsInBrowserContent()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FAssetData> AssetsData;
	ContentBrowserModule.Get().GetSelectedAssets(AssetsData);
	return AssetsData;
}

TArray<FString> MultiShaderFunc::GetSelectedFolderInBrowserContent()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FString> Folders;
	ContentBrowserModule.Get().GetSelectedFolders(Folders);
	return Folders;
}

void MultiShaderFunc::GetAssetDataInPaths(const TArray<FString>& Paths, TArray<FAssetData>& OutAssetData)
{
	// 从路径形成一个过滤器
	FARFilter Filter;
	// 如果为true，则PackagePath将递归
	Filter.bRecursivePaths = true;
	for (const FString& Path : Paths)
	{
		new (Filter.PackagePaths) FName(*Path);
	}
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
}

void MultiShaderFunc::DeleteDirectory(const FString& Dir)
{
	if (!Dir.IsEmpty() && FPaths::DirectoryExists(Dir))
	{
		UE_LOG(LogMultiShaderCooker, Display, TEXT("delete dir %s"), *Dir);
		IFileManager::Get().DeleteDirectory(*Dir, true, true);
	}
}

FString MultiShaderFunc::GenerateShaderCodeLibraryName(FString const& Name, bool bIsIterateSharedBuild)
{
	FString ActualName = (!bIsIterateSharedBuild) ? Name : Name + TEXT("_SC");
	return ActualName;
}

void MultiShaderFunc::InitCookShaderCollection(const FString& ShaderLibraryName, const bool& bIsNative,
                                               TArray<FString> GPlatformNames, TArray<ITargetPlatform*>GTargetPlatforms)
{
	//new一个FShaderLibrariesCollection
	SHADER_COOKER_CLASS::InitForCooking(bIsNative);
	//获取所有目标平台实例 并获取对应shader格式
	for (const auto& PlatformName : GPlatformNames)
	{
		ITargetPlatform* TargetPlatform = GetPlatformByName(PlatformName);
		GTargetPlatforms.AddUnique(TargetPlatform);
		TArray<FName> ShaderFormats = GetShaderFormatsByTargetPlatform(TargetPlatform);
		if (ShaderFormats.Num() > 0)
		{
			TArray<SHADER_COOKER_CLASS::FShaderFormatDescriptor> ShaderFormatsWithStableKeys = GetShaderFormatsWithStableKeys(ShaderFormats);
			SHADER_COOKER_CLASS::CookShaderFormats(ShaderFormatsWithStableKeys);
		}
	}
	//创建ShaderLibrary
	FShaderCodeLibrary::OpenLibrary(ShaderLibraryName, TEXT(""));
}

ITargetPlatform* MultiShaderFunc::GetPlatformByName(const FString& Name)
{
	static TMap<FString, ITargetPlatform*> PlatformNameMap;

	if (PlatformNameMap.Contains(Name))
		return *PlatformNameMap.Find(Name);

	ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
	const TArray<ITargetPlatform*>& TargetPlatforms = TPM.GetTargetPlatforms();
	ITargetPlatform* result = nullptr;
	for (ITargetPlatform* TargetPlatform : TargetPlatforms)
	{
		if (Name.Equals(TargetPlatform->PlatformName()))
		{
			result = TargetPlatform;
			PlatformNameMap.Add(Name, TargetPlatform);
			break;
		}
	}
	return result;
}

TArray<FName> MultiShaderFunc::GetShaderFormatsByTargetPlatform(const ITargetPlatform* TargetPlatform)
{
	TArray<FName> ShaderFormats;
	TargetPlatform->GetAllTargetedShaderFormats(ShaderFormats);
	return ShaderFormats;
}

TArray<FShaderCodeLibrary::FShaderFormatDescriptor> MultiShaderFunc::GetShaderFormatsWithStableKeys(
	const TArray<FName>& ShaderFormats, bool bNeedShaderStableKeys, bool bNeedsDeterministicOrder)
{
	TArray<SHADER_COOKER_CLASS::FShaderFormatDescriptor> ShaderFormatsWithStableKeys;
	for (const FName& Format : ShaderFormats)
	{
		SHADER_COOKER_CLASS::FShaderFormatDescriptor NewDesc;
		NewDesc.ShaderFormat = Format;
		NewDesc.bNeedsStableKeys = bNeedShaderStableKeys;
		NewDesc.bNeedsDeterministicOrder = bNeedsDeterministicOrder;
		ShaderFormatsWithStableKeys.Push(NewDesc);
	}
	return ShaderFormatsWithStableKeys;
}

void MultiShaderFunc::WaitCookerFinished()
{
	UPackage::WaitForAsyncFileWrites();
}

void MultiShaderFunc::WaitShaderCompilingComplete()
{
	if (GShaderCompilingManager)
	{
		UE_LOG(LogMultiShaderCooker, Display, TEXT("Waiting for shader compilation..."));
		while (GShaderCompilingManager->IsCompiling())
		{
			GShaderCompilingManager->ProcessAsyncResults(false, false);
			UE_LOG(LogMultiShaderCooker, Display, TEXT("Remaining Shader %d"), GShaderCompilingManager->GetNumRemainingJobs())
			FPlatformProcess::Sleep(0.5f);
		}
		GShaderCompilingManager->ProcessAsyncResults(false, false);
		UE_LOG(LogMultiShaderCooker, Display, TEXT("Shader Compilated!"));
	}
}

TArray<FString> MultiShaderFunc::FindCookedShaderLibByPlatform(const FString& PlatformName, const FString& Directory,
	bool bRecursive)
{
	TArray<FString> FoundFiles;
	auto GetMetalShaderFormatLambda = [](const FString& Directory, const FString& Extension, const bool bRecursive)
	{
		TArray<FString> FoundMetalLibFiles;
		if (bRecursive)
		{
			IFileManager::Get().FindFilesRecursive(FoundMetalLibFiles, *Directory, *Extension, true, false, false);
		}
		else
		{
			IFileManager::Get().FindFiles(FoundMetalLibFiles, *Directory, *Extension);
		}
		return FoundMetalLibFiles;
	};

	if (PlatformName.StartsWith(TEXT("IOS"), ESearchCase::IgnoreCase) || PlatformName.StartsWith(TEXT("Mac"), ESearchCase::IgnoreCase))
	{
		FoundFiles.Append(GetMetalShaderFormatLambda(Directory, TEXT("metallib"), bRecursive));
		FoundFiles.Append(GetMetalShaderFormatLambda(Directory, TEXT("metalmap"), bRecursive));
	}

	if (!FoundFiles.Num())
	{
		FoundFiles.Append(GetMetalShaderFormatLambda(Directory, TEXT("ushaderbytecode"), bRecursive));
	}
	for (auto& File : FoundFiles)
	{
		File = FPaths::Combine(Directory, File);
	}
	return FoundFiles;
}

void MultiShaderFunc::ShutDownShaderCollection(const FString& ShaderLibraryName, const bool& bIsNative,
                                               TArray<FString> GPlatformNames, TArray<ITargetPlatform*>GTargetPlatforms,
                                               const FString& StorageMetadataDir)
{
	for (const auto& TargetPlatform : GTargetPlatforms)
	{
		FString PlatformName = TargetPlatform->PlatformName();
		bool bSucceed = SaveShaderLibrary(TargetPlatform, ShaderLibraryName, StorageMetadataDir);
#if ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION <= 26
		if (bIsNative)
		{
			FString ShaderCodeDir = FPaths::Combine(StorageMetadataDir, PlatformName);
			bSucceed = bSucceed && FShaderCodeLibrary::PackageNativeShaderLibrary(ShaderCodeDir, GetShaderFormatsByTargetPlatform(TargetPlatform));
		}
#endif
		// rename StarterContent_SF_METAL.0.metallib to startercontent_sf_metal.0.metallib
		if (bSucceed && bIsNative && IsAppleMetalPlatform(TargetPlatform))
		{
			TArray<FString> FoundShaderLibs = FindCookedShaderLibByPlatform(PlatformName, FPaths::Combine(StorageMetadataDir, TargetPlatform->PlatformName()));
			for (const auto& ShaderLibrary : FoundShaderLibs)
			{
				if (ShaderLibrary.EndsWith(TEXT("metallib"), ESearchCase::IgnoreCase) || ShaderLibrary.EndsWith(TEXT("metalmap"), ESearchCase::IgnoreCase))
				{
					FString Path = FPaths::GetPath(ShaderLibrary);
					FString Name = FPaths::GetBaseFilename(ShaderLibrary, true);
					FString Extension = FPaths::GetExtension(ShaderLibrary, true);
					Name = FString::Printf(TEXT("%s%s"), *Name, *Extension);
					Name.ToLowerInline();
					if (!ShaderLibrary.EndsWith(Name, ESearchCase::CaseSensitive))
					{
						IFileManager::Get().Move(*FPaths::Combine(Path, Name), *ShaderLibrary, false);
					}
				}
			}
		}
	}
	FShaderCodeLibrary::CloseLibrary(ShaderLibraryName);
	FShaderCodeLibrary::Shutdown();
}

bool MultiShaderFunc::SaveShaderLibrary(const ITargetPlatform* TargetPlatform,
FString const& Name, const FString& SaveBaseDir)
{
	bool bSaved = false;
	FString ActualName = GenerateShaderCodeLibraryName(Name, false);
	FString BasePath = FPaths::ProjectContentDir();
	const FString ShaderCodeDir = FPaths::Combine(SaveBaseDir, TargetPlatform->PlatformName());
	const FString RootMetaDataPath = ShaderCodeDir / TEXT("Metadata") / TEXT("PipelineCaches");

	// Shader格式可以跨目标平台共享，用格式进行遍历
	TArray<FName> ShaderFormats;
	TargetPlatform->GetAllTargetedShaderFormats(ShaderFormats);
	if (ShaderFormats.Num() > 0)
	{
		if (ShaderFormats.Num() > 0)
		{
			FString TargetPlatformName = TargetPlatform->PlatformName();
			TArray<FString> PlatformSCLCSVPaths;
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 26
			FString ErrorString;
			bSaved = SHADER_COOKER_CLASS::SaveShaderLibraryWithoutChunking(TargetPlatform, FApp::GetProjectName(), ShaderCodeDir, RootMetaDataPath, PlatformSCLCSVPaths, ErrorString);
#else
			const TArray<TSet<FName>> ChunkAssignments;
			bSaved = FShaderCodeLibrary::SaveShaderCode(ShaderCodeDir, RootMetaDataPath, ShaderFormats, PlatformSCLCSVPaths, &ChunkAssignments);
#endif
		}
		return bSaved;
	}
	return bSaved;
}

bool MultiShaderFunc::IsAppleMetalPlatform(ITargetPlatform* TargetPlatform)
{
	bool bIsMatched = false;
	TArray<FString> ApplePlatforms = {TEXT("IOS"), TEXT("Mac"), TEXT("TVOS")};
	for (const auto& Platform : ApplePlatforms)
	{
		if (TargetPlatform->PlatformName().StartsWith(Platform, ESearchCase::IgnoreCase))
		{
			bIsMatched = true;
			break;
		}
	}
	return bIsMatched;
}

bool MultiShaderFunc::CookPackage(UPackage* Package, TArray<ITargetPlatform*> GTargetPlatforms, FString StorageCookedDir)
{
	bool bSuccessed = false;
	const FString LongPackageName = LongPackageNameToPackagePath(Package->GetPathName());
        TMap<FName, FString> PlatformSavePaths;
        for (const auto Platform : GTargetPlatforms)
        {
            FString SavePath = GetAssetCookedSavePath(StorageCookedDir, LongPackageName, Platform->PlatformName());
            PlatformSavePaths.Add(*Platform->PlatformName(), SavePath);
        }

        FString FakePackageName = FString(TEXT("Package ")) + LongPackageName;
        {
            if (!Package)
            {
                UE_LOG(LogMultiShaderCooker, Warning, TEXT("Cook %s UPackage is null!"), *LongPackageName);
            }
            if (Package && Package->HasAnyPackageFlags(PKG_EditorOnly))
            {
                UE_LOG(LogMultiShaderCooker, Warning, TEXT("Cook %s Failed! It is EditorOnly Package!"), *LongPackageName);
            }

            const bool bUnversioned = true;
            const bool CookLinkerDiff = false;
            const uint32 SaveFlags = GetCookSaveFlag(bUnversioned, false, CookLinkerDiff);
            const EObjectFlags CookedFlags = GetObjectFlagForCooked(Package);

#if ENGINE_MAJOR_VERSION > 4
            FName PackageFileName = Package->GetLoadedPath().GetPackageFName();
#else
            const FName PackageFileName = Package->FileName;
#endif
            if (PackageFileName.IsNone() && LongPackageName.IsEmpty())
                bSuccessed = false;

            const uint32 OriginalPackageFlags = Package->GetPackageFlags();

            struct FFilterEditorOnlyFlag
            {
                FFilterEditorOnlyFlag(UPackage* InPackage, ITargetPlatform* InPlatform)
                {
                    Package = InPackage;
                    Platform = InPlatform;
                    if (!Platform->HasEditorOnlyData())
                    {
                        Package->SetPackageFlags(PKG_FilterEditorOnly);
                    }
                    else
                    {
                        Package->ClearPackageFlags(PKG_FilterEditorOnly);
                    }
                }
                ~FFilterEditorOnlyFlag()
                {
                    if (!Platform->HasEditorOnlyData())
                    {
                        Package->ClearPackageFlags(PKG_FilterEditorOnly);
                    }
                }
                UPackage* Package;
                ITargetPlatform* Platform;
            };

            for (const auto& Platform : GTargetPlatforms)
            {
                FFilterEditorOnlyFlag SetPackageEditorOnlyFlag(Package, Platform);

                FString PackageName = PackageFileName.IsNone() ? LongPackageName : PackageFileName.ToString();
                FString CookedSavePath = *PlatformSavePaths.Find(*Platform->PlatformName());
                UE_LOG(LogMultiShaderCooker, Log, TEXT("Cook %s for %s"), *Package->GetName(), *Platform->PlatformName());

                if (!Platform->HasEditorOnlyData())
                {
                    Package->SetPackageFlags(PKG_FilterEditorOnly);
                }
                else
                {
                    Package->ClearPackageFlags(PKG_FilterEditorOnly);
                }

                PRAGMA_DISABLE_DEPRECATION_WARNINGS
                GIsCookerLoadingPackage = true;
                PRAGMA_ENABLE_DEPRECATION_WARNINGS

                FSavePackageResultStruct Result = GEditor->Save(
                    Package,
                    nullptr,
                    CookedFlags,
                    *CookedSavePath,
                    GError,
                    nullptr,
                    false,
                    false,
                    SaveFlags,
                    Platform,
                    FDateTime::MinValue(),
                    false,
                    /*DiffMap*/ nullptr);
                GIsCookerLoadingPackage = false;
                bSuccessed = Result == ESavePackageResult::Success;
            }

            Package->SetPackageFlagsTo(OriginalPackageFlags);
        }
	return bSuccessed;
}

uint32 MultiShaderFunc::GetCookSaveFlag(bool bUnversion, bool bStorageConcurrent,
	bool CookLinkerDiff)
{
	uint32 SaveFlags = SAVE_KeepGUID | SAVE_Async | SAVE_ComputeHash | (bUnversion ? SAVE_Unversioned : 0);

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 25
	// bool CookLinkerDiff = false;
	if (CookLinkerDiff)
	{
		SaveFlags |= SAVE_CompareLinker;
	}
#endif
	if (bStorageConcurrent)
	{
		SaveFlags |= SAVE_Concurrent;
	}
	return SaveFlags;
}

EObjectFlags MultiShaderFunc::GetObjectFlagForCooked(UPackage* Package)
{
	EObjectFlags CookedFlags = RF_Public;
	if (UWorld::FindWorldInPackage(Package))
	{
		CookedFlags = RF_NoFlags;
	}
	return CookedFlags;
}

FString MultiShaderFunc::ConvertToFullSandboxPath(const FString& FileName, bool bForWrite)
{
	const FString ProjectContentAbsir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    if (FileName.StartsWith(ProjectContentAbsir))
    {
        FString GameFileName = FileName;
        GameFileName.RemoveFromStart(ProjectContentAbsir);
        return FPaths::Combine(FApp::GetProjectName(), TEXT("Content"), GameFileName);
    }
    if (FileName.StartsWith(FPaths::EngineContentDir()))
    {
        FString EngineFileName = FileName;
        EngineFileName.RemoveFromStart(FPaths::EngineContentDir());
        return FPaths::Combine(TEXT("Engine/Content"), EngineFileName);
        ;
    }
    TArray<TSharedRef<IPlugin>> PluginsToRemap = IPluginManager::Get().GetEnabledPlugins();
    // Ideally this would be in the Sandbox File but it can't access the project or plugin
    if (PluginsToRemap.Num() > 0)
    {
        // Handle remapping of plugins
        for (const TSharedRef<IPlugin> Plugin : PluginsToRemap)
        {
            FString PluginContentDir;
            if (FPaths::IsRelative(FileName))
                PluginContentDir = Plugin->GetContentDir();
            else
                PluginContentDir = FPaths::ConvertRelativePathToFull(Plugin->GetContentDir());
            if (FileName.StartsWith(PluginContentDir))
            {
                FString SearchFor;
                SearchFor /= Plugin->GetName() / TEXT("Content");
                const int32 FoundAt = FileName.Find(SearchFor, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
                check(FoundAt != -1);
                // Strip off everything but <PluginName/Content/<remaing path to file>
                FString SnippedOffPath = FileName.RightChop(FoundAt);

                FString LoadingFrom;
                switch (Plugin->GetLoadedFrom())
                {
                case EPluginLoadedFrom::Engine:
                {
                    LoadingFrom = TEXT("Engine/Plugins");
                    break;
                }
                case EPluginLoadedFrom::Project:
                {
                    LoadingFrom = FPaths::Combine(FApp::GetProjectName(), TEXT("Plugins"));
                    break;
                }
                }

                return FPaths::Combine(LoadingFrom, SnippedOffPath);
            }
        }
    }

    return TEXT("");
}

UPackage* MultiShaderFunc::GetPackage(FName PackageName)
{
	if (PackageName == NAME_None)
	{
		return nullptr;
	}

	UPackage* Package = FindPackage(nullptr, *PackageName.ToString());
	if (Package)
	{
		Package->FullyLoad();
	}
	else
	{
		Package = LoadPackage(nullptr, *PackageName.ToString(), LOAD_None);
	}

	return Package;
}

FString MultiShaderFunc::LongPackageNameToPackagePath(const FString& InLongPackageName)
{
    if (InLongPackageName.Contains(TEXT(".")))
    {
        UE_LOG(LogMultiShaderCooker, Warning, TEXT("LongPackageNameToPackagePath %s alway is PackagePath!"), *InLongPackageName);
        return InLongPackageName;
    }
    FString AssetName;
    {
        int32 FoundIndex;
        InLongPackageName.FindLastChar('/', FoundIndex);
        if (FoundIndex != INDEX_NONE)
        {
        	AssetName = UKismetStringLibrary::GetSubstring(InLongPackageName, FoundIndex + 1, InLongPackageName.Len() - FoundIndex);
        }
    }
    FString OutPackagePath = InLongPackageName + TEXT(".") + AssetName;
    return OutPackagePath;
}

FString MultiShaderFunc::GetAssetCookedSavePath(const FString& BaseDir, const FString& PackageName,
	const FString& Platform)
{
	FString CookDir;
	FString Filename;
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (FPackageName::DoesPackageExist(PackageName, nullptr, &Filename, false))
	{
		const FString StandardFilename = FPaths::ConvertRelativePathToFull(Filename);
		FString SandboxFilename = ConvertToFullSandboxPath(*StandardFilename, true);
		CookDir = FPaths::Combine(BaseDir, Platform, SandboxFilename);
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	return CookDir;
}
