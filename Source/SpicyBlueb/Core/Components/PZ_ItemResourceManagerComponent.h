// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_ItemResourceManagerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBatchAssetsLoadedDelegate, TArray<FPrimaryAssetId> /* Loaded Ids */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSingleAssetLoadedDelegate, FPrimaryAssetId /* Loaded Id */);


/* Manages the lifetime of UPZ_ItemDataAsset resources */
UCLASS(ClassGroup=(ResourceLifetime), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_ItemResourceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZ_ItemResourceManagerComponent();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
};
