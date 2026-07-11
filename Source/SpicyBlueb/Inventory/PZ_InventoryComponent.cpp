// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventoryComponent.h"

#include "PZ_InventorySettings.h"
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

	if (const auto* InventorySettings = GetDefault<UPZ_InventorySettings>())
		MaxItemSlots = InventorySettings->MaxInventorySlots;
	else
		UE_LOG(LogTemp, Error, TEXT("Could not get inventory settings"));
	
	Items.SetNum(MaxItemSlots);
	
	// Broadcast item selection so UI defaults to zero index
	OnSlotSelectedDelegate.Broadcast(0);
}

void UPZ_InventoryComponent::SelectSlot(int32 Slot)
{
	if (Slot < 0 or Slot >= MaxItemSlots)
	{
		UE_LOG(LogTemp, Error, TEXT("Slot out of range"));
		return;
	}
	
	SelectedSlot = Slot;		
	OnSlotSelectedDelegate.Broadcast(Slot);
}

UPZ_ItemDataAsset* UPZ_InventoryComponent::GetItemData(int32 Slot) const
{
	if (Items.IsValidIndex(Slot))
		return Items[Slot].ItemData;
	
	return nullptr;
}

UPZ_ItemDataAsset* UPZ_InventoryComponent::GetSelectedItemData() const
{
	if (Items.IsValidIndex(SelectedSlot))
		return Items[SelectedSlot].ItemData;
	
	return nullptr;
	
}

bool UPZ_InventoryComponent::AddItem(FPrimaryAssetId ItemId)
{
	if (!ItemId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ItemId is invalid"));
		return false;
	}

	// find the first unoccupied slot
	int32 FirstSlot = -1;
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (!Items.IsValidIndex(i) or Items[i].IsOccupied)
			continue;

		FirstSlot = i;
		break;
	}

	if (FirstSlot <= -1)
		return false;

	// occupy the slot
	Items[FirstSlot].IsOccupied = true;
	Items[FirstSlot].AssetId = ItemId;
	Items[FirstSlot].ItemData = nullptr; // will be filled in by OnItemLoaded delegate

	// Note [Elias Brown]: Although the inventory is not accessing the item data, it is important that the item
	// is loaded for the entire stay within the inventory
	UAssetManager::Get().LoadPrimaryAsset(
		ItemId,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UPZ_InventoryComponent::OnItemLoaded, FirstSlot, ItemId));

	return true;
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
	Items[Slot].ItemData = nullptr;

	return AssetId;
}

void UPZ_InventoryComponent::OnItemLoaded(const int32 Slot, const FPrimaryAssetId AssetId) 
{
	UPZ_ItemDataAsset* ItemData = Cast<UPZ_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(AssetId));
	Items[Slot].ItemData = ItemData;
	OnItemLoadedDelegate.Broadcast(Slot, ItemData);
}
