// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_InventoryComponent.generated.h"

class UPZ_ItemDataAsset;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemLoaded, const int32 /* Slot */, const UPZ_ItemDataAsset* /* ItemId */)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const int32 /* Slot */)

USTRUCT()
struct FPZ_InventorySlot
{
	GENERATED_BODY()	
		
	UPROPERTY()
	FPrimaryAssetId AssetId;
	
	UPROPERTY()
	bool IsOccupied = false;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZ_InventoryComponent();
	virtual void BeginPlay() override;

	/* Adds an item to the first unoccupied slot */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItem(FPrimaryAssetId ItemId);
	
	/* Removes an item from the given slot. If there is no item at that slot, or if the slot index is invalid, it will return an invalid asset id. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FPrimaryAssetId TryPopItem(int32 Slot);
	
	FOnItemLoaded OnItemLoadedDelegate;
	FOnItemRemoved OnItemRemovedDelegate;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxItems = 2;
	
	void OnItemLoaded(const int32 Slot, FPrimaryAssetId AssetId) const;

	UPROPERTY()
	TArray<FPZ_InventorySlot> Items;
};
