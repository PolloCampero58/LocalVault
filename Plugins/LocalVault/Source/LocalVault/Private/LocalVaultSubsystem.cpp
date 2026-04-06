// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#include "LocalVaultSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Utilities/FabAssetsCache.h"
#include "Misc/Paths.h"

void ULocalVaultSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadLocalDatabase();
}

void ULocalVaultSubsystem::Deinitialize()
{
	SaveLocalDatabase();
	Super::Deinitialize();
}

void ULocalVaultSubsystem::RefreshOwnedAssets()
{
	UE_LOG(LogTemp, Log, TEXT("LocalVault: Syncing assets via FabAssetsCache..."));
	
	// Remove any old mock items
	LocalAssetDatabase.Remove(TEXT("fab_empty"));

	// Use native Epic Fab module to populate local cache
	TArray<FString> CachedFabAssets = FFabAssetsCache::GetCachedAssets();
	
	UE_LOG(LogTemp, Log, TEXT("LocalVault: Found %d assets in the local Fab cache."), CachedFabAssets.Num());
	
	if (CachedFabAssets.Num() > 0)
	{
		for (const FString& AssetId : CachedFabAssets)
		{
			if (!LocalAssetDatabase.Contains(AssetId))
			{
				FLocalVaultAssetInfo NewAsset;
				NewAsset.AssetId = AssetId;
				
				// Extract directory name for a readable title
				FString CacheFilePath = FFabAssetsCache::GetCachedFile(AssetId);
				NewAsset.Title = FPaths::GetBaseFilename(CacheFilePath); 
				if (NewAsset.Title.IsEmpty())
				{
					NewAsset.Title = AssetId;
				}
				
				NewAsset.Author = TEXT("Epic Games / Fab");
				NewAsset.bIsDownloaded = true;
				NewAsset.LocalPath = CacheFilePath;
				NewAsset.CustomTags = { TEXT("FabCache") };
				LocalAssetDatabase.Add(AssetId, NewAsset);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalVault: No cached Fab assets found. Please download assets via the Fab window in Unreal Editor first to populate the cache."));
	}

	SaveLocalDatabase();
	OnVaultUpdated.Broadcast();
}

void ULocalVaultSubsystem::AddCustomTagToAsset(const FString& InAssetId, const FString& InTag)
{
	if (FLocalVaultAssetInfo* AssetInfo = LocalAssetDatabase.Find(InAssetId))
	{
		AssetInfo->CustomTags.AddUnique(InTag);
		SaveLocalDatabase();
		OnVaultUpdated.Broadcast();
	}
}

void ULocalVaultSubsystem::RemoveCustomTagFromAsset(const FString& InAssetId, const FString& InTag)
{
	if (FLocalVaultAssetInfo* AssetInfo = LocalAssetDatabase.Find(InAssetId))
	{
		if (AssetInfo->CustomTags.Remove(InTag) > 0)
		{
			SaveLocalDatabase();
			OnVaultUpdated.Broadcast();
		}
	}
}

TArray<FLocalVaultAssetInfo> ULocalVaultSubsystem::GetAllTrackedAssets() const
{
	TArray<FLocalVaultAssetInfo> OutAssets;
	LocalAssetDatabase.GenerateValueArray(OutAssets);
	return OutAssets;
}

TArray<FString> ULocalVaultSubsystem::GetAllUniqueTags() const
{
	TSet<FString> UniqueTags;
	for (const auto& Pair : LocalAssetDatabase)
	{
		UniqueTags.Append(Pair.Value.CustomTags);
	}
	
	TArray<FString> OutTags = UniqueTags.Array();
	OutTags.Sort();
	return OutTags;
}

void ULocalVaultSubsystem::LoadLocalDatabase()
{
	FString DatabasePath = FPaths::ProjectSavedDir() / TEXT("LocalVault/Database.json");
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *DatabasePath))
	{
		TSharedPtr<FJsonObject> RootObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(Reader, RootObject) && RootObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* AssetsArray;
			if (RootObject->TryGetArrayField(TEXT("assets"), AssetsArray))
			{
				for (TSharedPtr<FJsonValue> AssetValue : *AssetsArray)
				{
					TSharedPtr<FJsonObject> AssetObject = AssetValue->AsObject();
					if (AssetObject.IsValid())
					{
						FLocalVaultAssetInfo LoadedAsset;
						if (FJsonObjectConverter::JsonObjectToUStruct(AssetObject.ToSharedRef(), &LoadedAsset, 0, 0))
						{
							LocalAssetDatabase.Add(LoadedAsset.AssetId, LoadedAsset);
						}
					}
				}
			}
		}
	}
}

void ULocalVaultSubsystem::SaveLocalDatabase()
{
	FString DatabasePath = FPaths::ProjectSavedDir() / TEXT("LocalVault/Database.json");
	
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AssetsArray;
	
	for (const auto& Pair : LocalAssetDatabase)
	{
		TSharedPtr<FJsonObject> AssetObject = MakeShared<FJsonObject>();
		if (FJsonObjectConverter::UStructToJsonObject(FLocalVaultAssetInfo::StaticStruct(), &Pair.Value, AssetObject.ToSharedRef(), 0, 0))
		{
			AssetsArray.Add(MakeShared<FJsonValueObject>(AssetObject));
		}
	}
	
	RootObject->SetArrayField(TEXT("assets"), AssetsArray);
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		FFileHelper::SaveStringToFile(JsonString, *DatabasePath);
	}
}