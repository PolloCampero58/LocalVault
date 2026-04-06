// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LocalVaultSettings.generated.h"

/**
 * Configuration settings for the LocalVault plugin.
 */
UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="LocalVault"))
class LOCALVAULT_API ULocalVaultSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	ULocalVaultSettings();

	/** If true, automatically checks for updates for your locally stored assets when opening the project. */
	UPROPERTY(Config, EditAnywhere, Category="Network")
	bool bAutoCheckForUpdates;
};