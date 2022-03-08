// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiShaderLibrary.h"
#include "FMultiShaderCooker.h"

#define LOCTEXT_NAMESPACE "FMultiShaderLibraryModule"

void FMultiShaderLibraryModule::StartupModule()
{
}

void FMultiShaderLibraryModule::ShutdownModule()
{
}

const TSharedRef<FMultiShaderCooker> MultiShaderCooker(new FMultiShaderCooker());

// Cheat command to load all assets of a given type
static FAutoConsoleCommand CookChosenAssets(
    TEXT("CookChoosedAssets"),
    TEXT("Cook Choosed Assets in the Content Broweser"),
    FConsoleCommandDelegate::CreateSP(MultiShaderCooker, &FMultiShaderCooker::CreateShaderCodeLibrary),
    ECVF_Default);

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMultiShaderLibraryModule, MultiShaderLibrary)
