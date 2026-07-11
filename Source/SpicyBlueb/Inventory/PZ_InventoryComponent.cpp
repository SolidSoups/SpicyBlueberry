// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventoryComponent.h"

#include "PZ_ItemData.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"

UPZ_InventoryComponent::UPZ_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	constexpr int32 MAX_ITEMS = 2;	
	Items.SetNum(MAX_ITEMS);
}

void UPZ_InventoryComponent::AddItem(FPrimaryAssetId ItemId)
{
	// find the first unoccupied slot
	int32 FirstSlot = -1;
	for (int32 i=0; i < Items.Num(); i++)
	{
		if (!Items.IsValidIndex(i) or Items[i].IsOccupied)	
			continue;
		
		FirstSlot = i;
		break;
	}
	
	if (FirstSlot <= -1)
		return;

	// occupy the slot
	Items[FirstSlot].IsOccupied = true;
	Items[FirstSlot].AssetId = ItemId;
	
	// Note [Elias Brown]: Although the inventory is not accessing the item data, it is important that the item
	// is loaded for the entire stay within the inventory
	UAssetManager::Get().LoadPrimaryAsset(
		ItemId,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UPZ_InventoryComponent::OnItemLoaded,  FirstSlot, ItemId));
}

FPrimaryAssetId UPZ_InventoryComponent::TryPopItem(int32 Slot)
{
	if (!Items.IsValidIndex(Slot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove an item from an unused slot (%i)"), Slot);
		return FPrimaryAssetId{};
	}

	// release our asset ref count
	FPrimaryAssetId AssetId = Items[Slot].AssetId;
	UAssetManager::Get().UnloadPrimaryAsset(AssetId);
	
	OnItemRemovedDelegate.Broadcast(Slot);
	
	// Reset slot to blank state
	Items[Slot].IsOccupied = false;
	Items[Slot].AssetId = FPrimaryAssetId{};

	return AssetId;
}

void UPZ_InventoryComponent::OnItemLoaded(const int32 Slot, const FPrimaryAssetId AssetId) const
{
	UPZ_ItemDataAsset* ItemData = Cast<UPZ_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(AssetId));
	OnItemLoadedDelegate.Broadcast(Slot, ItemData);
}
