// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PZ_AssetManagerSubsystem.generated.h"

DECLARE_DELEGATE_OneParam(FOnAssetLoadedDelegate, FPrimaryAssetId /* LoadedAssetId */);
DECLARE_DELEGATE_OneParam(FOnAssetBatchLoadedDelegate, TArray<FPrimaryAssetId> /* LoadedAssetIds */);

/* A wrapper for UAssetManager which tracks refcounts internally to ensure that an asset is unable to unload until everyone has released it. 
  
 This system only tracks refcounts. To get an object, use UAssetManager::Get().GetPrimaryAssetObject(..)
 */
UCLASS()
class SPICYBLUEB_API UPZ_AssetManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Deinitialize() override;

	void LoadSingleAsset(FPrimaryAssetId AssetId, FOnAssetLoadedDelegate Callback = FOnAssetLoadedDelegate());
	void LoadBatchAssets(TArray<FPrimaryAssetId> AssetIds, FOnAssetBatchLoadedDelegate Callback = FOnAssetBatchLoadedDelegate());
	void ReleaseSingleAsset(FPrimaryAssetId AssetId);
	void ReleaseBatchAssets(TArray<FPrimaryAssetId> AssetIds);

private:
	void OnSingleAssetLoaded(FPrimaryAssetId PrimaryAssetId, FOnAssetLoadedDelegate Callback);
	void OnBatchAssetsLoaded(TArray<FPrimaryAssetId> PrimaryAssetIds, FOnAssetBatchLoadedDelegate Callback);
	
	TMap<FPrimaryAssetId, TWeakObjectPtr<UObject>> LoadedObjects;
	TMap<FPrimaryAssetId, int32> AssetRefCounts;	
};
