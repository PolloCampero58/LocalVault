// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "LocalVault.h"
#include "ToolMenus.h"
#include "SLocalVaultBrowser.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

static const FName LocalVaultTabName("LocalVault");

#define LOCTEXT_NAMESPACE "FLocalVaultModule"

void FLocalVaultModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LocalVaultTabName, FOnSpawnTab::CreateRaw(this, &FLocalVaultModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FLocalVaultTabTitle", "LocalVault Browser"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLocalVaultModule::RegisterMenus));
}

void FLocalVaultModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LocalVaultTabName);
}

TSharedRef<SDockTab> FLocalVaultModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SLocalVaultBrowser)
		];
}

void FLocalVaultModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(LocalVaultTabName);
}

void FLocalVaultModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntry(
				"LocalVault",
				LOCTEXT("LocalVaultMenuLabel", "LocalVault"),
				LOCTEXT("LocalVaultMenuTooltip", "Open the LocalVault Package Manager"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FLocalVaultModule::PluginButtonClicked)),
				EUserInterfaceActionType::Button
			);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLocalVaultModule, LocalVault)