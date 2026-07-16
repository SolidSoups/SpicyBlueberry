// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_AssetManagerSubsystem.h"

#include "Engine/AssetManager.h"

void UPZ_AssetManagerSubsystem::Deinitialize()
{
	TArray<FPrimaryAssetId> AssetsToUnload;
	AssetRefCounts.GetKeys(AssetsToUnload);
	UAssetManager::Get().UnloadPrimaryAssets(AssetsToUnload);
	AssetRefCounts.Reset();

	Super::Deinitialize();
}

void UPZ_AssetManagerSubsystem::LoadSingleAsset(FPrimaryAssetId AssetId, FOnAssetLoadedDelegate Callback)
{
	if (!ensureMsgf(AssetId.IsValid(), TEXT("LoadSingleAsset called for invalid asset")))
	{
		return;
	}

	AssetRefCounts.FindOrAdd(AssetId)++;
	UAssetManager::Get().LoadPrimaryAsset(AssetId, TArray<FName>(),
	                                      FStreamableDelegate::CreateUObject(
		                                      this, &UPZ_AssetManagerSubsystem::OnSingleAssetLoaded, AssetId, Callback));
}

void UPZ_AssetManagerSubsystem::LoadBatchAssets(TArray<FPrimaryAssetId> AssetIds, FOnAssetBatchLoadedDelegate Callback)
{
	TArray<FPrimaryAssetId> ValidAssetIds;
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (!ensureMsgf(AssetId.IsValid(), TEXT("LoadBatchAsset called for an invalid asset")))
			continue;

		ValidAssetIds.Add(AssetId);
	}

	for (auto& AssetId : ValidAssetIds)
	{
		AssetRefCounts.FindOrAdd(AssetId)++;
	}
	UAssetManager::Get().LoadPrimaryAssets(
		ValidAssetIds,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UPZ_AssetManagerSubsystem::OnBatchAssetsLoaded, ValidAssetIds, Callback));
}

void UPZ_AssetManagerSubsystem::ReleaseSingleAsset(FPrimaryAssetId AssetId)
{
	int32* RefCount = AssetRefCounts.Find(AssetId);
	if (!ensureMsgf(RefCount, TEXT("ReleaseSingleAsset called for '%s' with no matching load"), *AssetId.ToString()))
	{
		return; // nobody ever loaded this asset
	}

	if (--(*RefCount) <= 0)
	{
		UAssetManager::Get().UnloadPrimaryAsset(AssetId);
		AssetRefCounts.Remove(AssetId);
	}
}

void UPZ_AssetManagerSubsystem::ReleaseBatchAssets(TArray<FPrimaryAssetId> AssetIds)
{
	TArray<FPrimaryAssetId> AssetsToUnload;
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		int32* RefCount = AssetRefCounts.Find(AssetId);
		if (!ensureMsgf(RefCount, TEXT("ReleaseBatchAsset called for '%s' with no matching load"), *AssetId.ToString()))
			continue;

		if (--(*RefCount) <= 0)
		{
			AssetsToUnload.Add(AssetId);
			AssetRefCounts.Remove(AssetId);
		}
	}

	UAssetManager::Get().UnloadPrimaryAssets(AssetsToUnload);
}

void UPZ_AssetManagerSubsystem::OnSingleAssetLoaded(FPrimaryAssetId PrimaryAssetId, FOnAssetLoadedDelegate Callback)
{
	Callback.ExecuteIfBound(PrimaryAssetId);
}

void UPZ_AssetManagerSubsystem::OnBatchAssetsLoaded(TArray<FPrimaryAssetId> PrimaryAssetIds, FOnAssetBatchLoadedDelegate Callback)
{
	Callback.ExecuteIfBound(PrimaryAssetIds);
}
