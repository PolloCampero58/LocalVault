// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "LocalVaultSubsystem.h"

class SLocalVaultBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLocalVaultBrowser) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> OnGenerateRowForAsset(TSharedPtr<FLocalVaultAssetInfo> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void RefreshAssetList();
	FReply OnRefreshClicked();

	void OnAddTagTextChanged(const FText& InText);
	FReply OnAddTagClicked();
	void OnVaultUpdated();

	void OnSearchTextChanged(const FText& InFilterText);

	TSharedRef<ITableRow> OnGenerateRowForTag(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnTagSelectionChanged(TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo);
	void RefreshTagList();

private:
	TArray<TSharedPtr<FLocalVaultAssetInfo>> FilteredAssets;
	TSharedPtr<SListView<TSharedPtr<FLocalVaultAssetInfo>>> AssetListView;
	
	TArray<TSharedPtr<FString>> FilteredTags;
	TSharedPtr<SListView<TSharedPtr<FString>>> TagListView;
	
	TSharedPtr<class SEditableTextBox> TagTextBox;
	FString CurrentSearchText;
};