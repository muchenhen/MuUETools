// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ContentBrowserModule.h"
#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE(FOnActionbutClick);


class FMultiShaderLibraryModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

