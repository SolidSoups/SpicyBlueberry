// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_InventoryComponent.generated.h"

class UPZ_ItemData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemLoaded, FPrimaryAssetId, UPZ_ItemData*)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, FPrimaryAssetId)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZ_InventoryComponent();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItem(FPrimaryAssetId ItemId);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	TOptional<FPrimaryAssetId> TryPop(int32 Slot);
	
	FOnItemLoaded OnItemLoadedDelegate;
	FOnItemRemoved OnItemRemovedDelegate;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxItems = 2;
	
	void OnItemLoaded(FPrimaryAssetId ItemId);

	UPROPERTY()
	TArray<FPrimaryAssetId> Items;
};
