// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "LocalVaultSettings.h"

ULocalVaultSettings::ULocalVaultSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName  = TEXT("LocalVault");

	bAutoCheckForUpdates = true;
}