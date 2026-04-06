// Copyright (c) 2026 GregOrigin. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "HttpFwd.h"
#include "LocalVaultSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLocalVaultUpdated);

USTRUCT(BlueprintType)
struct FLocalVaultAssetInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	FString AssetId;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	FString Title;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	FString Author;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	FString LocalPath;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	TArray<FString> CustomTags;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	bool bIsDownloaded = false;

	UPROPERTY(BlueprintReadOnly, Category="LocalVault")
	FString EngineVersionCompatibility;
};

/**
 * Subsystem responsible for managing the local database and syncing with the Epic account for LocalVault.
 */
UCLASS()
class LOCALVAULT_API ULocalVaultSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FSimpleMulticastDelegate OnVaultUpdated;

	/** Refreshes the local database of owned Fab assets. */
	UFUNCTION(BlueprintCallable, Category="LocalVault")
	void RefreshOwnedAssets();

	/** Adds a custom tag to a specific asset. */
	UFUNCTION(BlueprintCallable, Category="LocalVault")
	void AddCustomTagToAsset(const FString& InAssetId, const FString& InTag);

	/** Removes a custom tag from a specific asset. */
	UFUNCTION(BlueprintCallable, Category="LocalVault")
	void RemoveCustomTagFromAsset(const FString& InAssetId, const FString& InTag);

	/** Returns all assets currently tracked in the local database. */
	UFUNCTION(BlueprintCallable, Category="LocalVault")
	TArray<FLocalVaultAssetInfo> GetAllTrackedAssets() const;

	/** Returns a sorted list of all unique custom tags applied across the entire local database. */
	UFUNCTION(BlueprintCallable, Category="LocalVault")
	TArray<FString> GetAllUniqueTags() const;

private:
	void LoadLocalDatabase();
	void SaveLocalDatabase();

private:
	TMap<FString, FLocalVaultAssetInfo> LocalAssetDatabase;
};