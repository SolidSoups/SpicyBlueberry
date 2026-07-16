#pragma once
#include "PZ_AssetManagerSubsystem.h"

class UWorld;

// Simplifies management of assets lifecycle. Use Load and LoadBatch to load assets, and Release/ReleaseAll to release them again
struct FPZ_ItemAssetRequester
{
	void Load(UWorld* World, FPrimaryAssetId AssetId, FOnAssetLoadedDelegate Callback = FOnAssetLoadedDelegate());
	void LoadBatch(UWorld* World, TArray<FPrimaryAssetId> AssetIds, FOnAssetBatchLoadedDelegate Callback = FOnAssetBatchLoadedDelegate());
	void Release(UWorld* World, FPrimaryAssetId AssetId);
	void ReleaseAll(UWorld* World);
	
private:
	TArray<FPrimaryAssetId> RequestedIds;
};
