#include "PZ_ItemAssetRequester.h"

#include "Engine/World.h"

void FPZ_ItemAssetRequester::Load(UWorld* World, FPrimaryAssetId AssetId, FOnAssetLoadedDelegate Callback)
{
	if (auto* Subsystem = World ? World->GetSubsystem<UPZ_AssetManagerSubsystem>() : nullptr)
	{
		RequestedIds.Add(AssetId);
		Subsystem->LoadSingleAsset(AssetId, Callback);
	}
}

void FPZ_ItemAssetRequester::LoadBatch(UWorld* World, TArray<FPrimaryAssetId> AssetIds,
                                       FOnAssetBatchLoadedDelegate Callback)
{
	if (auto* Subsystem = World ? World->GetSubsystem<UPZ_AssetManagerSubsystem>() : nullptr)
	{
		RequestedIds.Append(AssetIds);
		Subsystem->LoadBatchAssets(AssetIds, Callback);
	}
}

void FPZ_ItemAssetRequester::Release(UWorld* World, FPrimaryAssetId AssetId)
{
	int32 Index = RequestedIds.Find(AssetId);
	if (Index == INDEX_NONE)
		return;

	RequestedIds.RemoveAtSwap(Index);
	if (auto* Subsystem = World ? World->GetSubsystem<UPZ_AssetManagerSubsystem>() : nullptr)
	{
		RequestedIds.Remove(AssetId);
		Subsystem->ReleaseSingleAsset(AssetId);
	}
}

void FPZ_ItemAssetRequester::ReleaseAll(UWorld* World)
{
	if (RequestedIds.IsEmpty())
		return;

	if (auto* Subsystem = World ? World->GetSubsystem<UPZ_AssetManagerSubsystem>() : nullptr)
	{
		Subsystem->ReleaseBatchAssets(RequestedIds);
	}
	
	RequestedIds.Reset();
}
