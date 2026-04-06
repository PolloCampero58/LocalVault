// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "SLocalVaultBrowser.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "EditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "LocalVaultBrowser"

void SLocalVaultBrowser::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor(0.015f, 0.025f, 0.045f, 1.0f)) // Deep Dark Blue Background
		.Padding(4)
		[
			SNew(SVerticalBox)
			
			// Branding Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8, 8, 8, 12)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BrandTitle", "LOCAL VAULT"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
					.ColorAndOpacity(FLinearColor(0.0f, 0.7f, 1.0f, 1.0f)) // Cyan
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(12, 0, 0, 0)
				[
					SNew(SColorBlock)
					.Color(FLinearColor(0.0f, 0.4f, 0.6f, 0.3f)) // Subtle cyan line
					.Size(FVector2D(100.0f, 1.0f))
				]
			]

			// Toolbar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5, 0, 5, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SSearchBox)
					.HintText(LOCTEXT("SearchHint", "Search offline library (title, author, tag)..."))
					.OnTextChanged(this, &SLocalVaultBrowser::OnSearchTextChanged)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8, 0, 0, 0)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					.Text(LOCTEXT("RefreshBtn", "Sync with Epic"))
					.OnClicked(this, &SLocalVaultBrowser::OnRefreshClicked)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(15, 0, 0, 0)
				[
					SAssignNew(TagTextBox, SEditableTextBox)
					.HintText(LOCTEXT("TagHint", "New Tag..."))
					.MinDesiredWidth(100.0f)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5, 0, 0, 0)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
					.Text(LOCTEXT("AddTagBtn", "+ Add Tag"))
					.OnClicked(this, &SLocalVaultBrowser::OnAddTagClicked)
				]
			]
			
			// Main Content Splitter
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(5, 0, 5, 5)
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
				
				// Sidebar (Collections & Wishlists)
				+ SSplitter::Slot()
				.Value(0.2f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					.Padding(4)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(4, 4, 4, 8)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CollectionsHeader", "Collections & Wishlists"))
							.Font(FAppStyle::GetFontStyle("BoldFont"))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SAssignNew(TagListView, SListView<TSharedPtr<FString>>)
							.ListItemsSource(&FilteredTags)
							.OnGenerateRow(this, &SLocalVaultBrowser::OnGenerateRowForTag)
							.OnSelectionChanged(this, &SLocalVaultBrowser::OnTagSelectionChanged)
							.SelectionMode(ESelectionMode::Single)
						]
					]
				]
				
				// Main List Area
				+ SSplitter::Slot()
				.Value(0.8f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					.Padding(2)
					[
						SAssignNew(AssetListView, SListView<TSharedPtr<FLocalVaultAssetInfo>>)
						.ListItemsSource(&FilteredAssets)
						.OnGenerateRow(this, &SLocalVaultBrowser::OnGenerateRowForAsset)
						.SelectionMode(ESelectionMode::Multi)
						.ClearSelectionOnClick(false)
					]
				]
			]
		]
	];

	if (GEditor)
	{
		if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
		{
			Subsystem->OnVaultUpdated.AddRaw(this, &SLocalVaultBrowser::OnVaultUpdated);
		}
	}

	RefreshTagList();
	RefreshAssetList();
}

TSharedRef<ITableRow> SLocalVaultBrowser::OnGenerateRowForAsset(TSharedPtr<FLocalVaultAssetInfo> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString DownloadStatus = InItem->bIsDownloaded ? TEXT("● LOCAL") : TEXT("○ CLOUD");
	FLinearColor StatusColor = InItem->bIsDownloaded ? FLinearColor(0.0f, 0.8f, 1.0f, 1.0f) : FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

	TSharedRef<SHorizontalBox> TagsBox = SNew(SHorizontalBox);
	for (const FString& AssetTag : InItem->CustomTags)
	{
		FString AssetId = InItem->AssetId;
		FString TagToRemove = AssetTag;

		TagsBox->AddSlot()
		.AutoWidth()
		.Padding(0, 0, 6, 0)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.5f, 0.8f, 0.2f)) // Subtle cyan pill background
			.Padding(FMargin(8.0f, 2.0f))
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(AssetTag))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.8f, 1.0f, 1.0f)))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 0, 0, 0)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
					.ContentPadding(FMargin(2.0f, 0.0f))
					.Cursor(EMouseCursor::Hand)
					.OnClicked_Lambda([AssetId, TagToRemove]() -> FReply
					{
						if (GEditor)
						{
							if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
							{
								Subsystem->RemoveCustomTagFromAsset(AssetId, TagToRemove);
							}
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("x")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
						.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.4f, 0.4f, 1.0f)))
					]
				]
			]
		];
	}

	return SNew(STableRow<TSharedPtr<FLocalVaultAssetInfo>>, OwnerTable)
		.Padding(FMargin(4.0f, 6.0f))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.5f))
			.Padding(6)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.15f)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 15, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(DownloadStatus))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(StatusColor)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->Title))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.2f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(InItem->Author))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				[
					TagsBox
				]
			]
		];
}

void SLocalVaultBrowser::RefreshAssetList()
{
	FilteredAssets.Empty();

	if (GEditor)
	{
		if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
		{
			TArray<FLocalVaultAssetInfo> Assets = Subsystem->GetAllTrackedAssets();
			for (const FLocalVaultAssetInfo& AssetInfo : Assets)
			{
				bool bInclude = true;

				if (!CurrentSearchText.IsEmpty())
				{
					bInclude = AssetInfo.Title.Contains(CurrentSearchText) ||
					           AssetInfo.Author.Contains(CurrentSearchText);

					if (!bInclude)
					{
						for (const FString& AssetTag : AssetInfo.CustomTags)
						{
							if (AssetTag.Contains(CurrentSearchText))
							{
								bInclude = true;
								break;
							}
						}
					}
				}

				if (bInclude)
				{
					FilteredAssets.Add(MakeShared<FLocalVaultAssetInfo>(AssetInfo));
				}
			}
		}
	}

	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}
}

FReply SLocalVaultBrowser::OnRefreshClicked()
{
	if (GEditor)
	{
		if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
		{
			Subsystem->RefreshOwnedAssets();
			RefreshAssetList();
		}
	}
	return FReply::Handled();
}

FReply SLocalVaultBrowser::OnAddTagClicked()
{
	FString TagToAdd;
	if (TagTextBox.IsValid())
	{
		TagToAdd = TagTextBox->GetText().ToString();
	}

	if (TagToAdd.IsEmpty()) return FReply::Handled();

	if (AssetListView.IsValid() && GEditor)
	{
		if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
		{
			TArray<TSharedPtr<FLocalVaultAssetInfo>> SelectedItems = AssetListView->GetSelectedItems();
			for (auto& Item : SelectedItems)
			{
				Subsystem->AddCustomTagToAsset(Item->AssetId, TagToAdd);
			}
			
			if (TagTextBox.IsValid())
			{
				TagTextBox->SetText(FText::GetEmpty());
			}
		}
	}
	return FReply::Handled();
}

void SLocalVaultBrowser::OnVaultUpdated()
{
	RefreshTagList();
	RefreshAssetList();
}

void SLocalVaultBrowser::OnSearchTextChanged(const FText& InFilterText)
{
	CurrentSearchText = InFilterText.ToString();
	RefreshAssetList();
}

TSharedRef<ITableRow> SLocalVaultBrowser::OnGenerateRowForTag(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		.Padding(FMargin(4.0f, 6.0f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem))
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f)))
		];
}

void SLocalVaultBrowser::OnTagSelectionChanged(TSharedPtr<FString> InItem, ESelectInfo::Type SelectInfo)
{
	if (InItem.IsValid())
	{
		if (*InItem == TEXT("All Assets"))
		{
			CurrentSearchText = TEXT("");
		}
		else
		{
			CurrentSearchText = *InItem;
		}
		RefreshAssetList();
	}
}

void SLocalVaultBrowser::RefreshTagList()
{
	FilteredTags.Empty();
	FilteredTags.Add(MakeShared<FString>(TEXT("All Assets")));

	if (GEditor)
	{
		if (ULocalVaultSubsystem* Subsystem = GEditor->GetEditorSubsystem<ULocalVaultSubsystem>())
		{
			TArray<FString> Tags = Subsystem->GetAllUniqueTags();
			for (const FString& AssetTag : Tags)
			{
				FilteredTags.Add(MakeShared<FString>(AssetTag));
			}
		}
	}

	if (TagListView.IsValid())
	{
		TagListView->RequestListRefresh();
	}
}

#undef LOCTEXT_NAMESPACE