// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_InventoryComponent.generated.h"

class UPZ_ItemDataAsset;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemLoaded, const int32 /* Slot */, const UPZ_ItemDataAsset* /* ItemId */)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const int32 /* Slot */)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotSelected, const int32 /* Slot */)

USTRUCT()
struct FPZ_InventorySlot
{
	GENERATED_BODY()	
		
	UPROPERTY()
	FPrimaryAssetId AssetId{};
	
	UPROPERTY()
	UPZ_ItemDataAsset* ItemData = nullptr;
	
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
	
	/* Select a slot in the inventory */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetSelectedSlot(int32 Slot);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 GetMaxSlots() const { return Items.Num();}
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 GetSelectedSlot() const { return SelectedSlot;}
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UPZ_ItemDataAsset* GetItemData(int32 Slot) const;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UPZ_ItemDataAsset* GetSelectedItemData() const;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool HasItem(FPrimaryAssetId ItemId) const;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 FindSlotWithItem(FPrimaryAssetId ItemId) const;

	/* Adds an item to the first unoccupied slot */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool AddItem(FPrimaryAssetId ItemId);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool TryRemoveItem(FPrimaryAssetId ItemId);
	
	/* Tries to remove an item from the given Slot. If successful, you are given a valid Assed ID handle for the removed item. */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FPrimaryAssetId TryPopItemSlot(int32 Slot);
	
	FOnItemLoaded OnItemLoadedDelegate;
	FOnItemRemoved OnItemRemovedDelegate;
	FOnSlotSelected OnSlotSelectedDelegate;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxItemSlots = 2; // default here only applies if settings not found
	
	void OnItemLoaded(const int32 Slot, FPrimaryAssetId AssetId);

	UPROPERTY()
	TArray<FPZ_InventorySlot> Items;
	
	TBitArray<> IsSlotLoading;
	
	int32 SelectedSlot = 0;
	
private:
	void DeleteSlot(int32 Slot);
};