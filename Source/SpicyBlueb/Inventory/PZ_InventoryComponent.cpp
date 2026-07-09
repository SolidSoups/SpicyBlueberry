// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventoryComponent.h"

#include "PZ_ItemData.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"

UPZ_InventoryComponent::UPZ_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_InventoryComponent::AddItem(FPrimaryAssetId ItemId)
{
	if (Items.Num() >= MaxItems)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player is holding max items already"));
		return;
	}

	Items.Add(ItemId);

	UAssetManager::Get().LoadPrimaryAsset(
		ItemId,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UPZ_InventoryComponent::OnItemLoaded, ItemId));
}

TOptional<FPrimaryAssetId> UPZ_InventoryComponent::TryPop(int32 Slot)
{
	if (Slot < 0 or Slot >= Items.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove an item from an unused slot (%i)"), Slot);
		return TOptional<FPrimaryAssetId>();
	}

	FPrimaryAssetId ItemId = Items[Slot];
	Items.RemoveAt(Slot); // TODO [Elias Brown]: Removing an item shouldn't change ahead elements indices

	// DEBUG
	UPZ_ItemData* Data = Cast<UPZ_ItemData>(UAssetManager::Get().GetPrimaryAssetObject(ItemId));
	if (IsValid(Data))
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green,
		                                 FString::Printf(
			                                 TEXT("Removed item (slot=%i): %s"), Slot, *Data->DisplayName.ToString()));
	}

	UAssetManager::Get().UnloadPrimaryAsset(ItemId);

	OnItemRemovedDelegate.Broadcast(ItemId);

	return ItemId;
}

void UPZ_InventoryComponent::OnItemLoaded(FPrimaryAssetId ItemId)
{
	UPZ_ItemData* Data = Cast<UPZ_ItemData>(UAssetManager::Get().GetPrimaryAssetObject(ItemId));
	if (IsValid(Data))
	{
		// TODO [Elias Brown]: This is not a valid way to find a slot if we have duplicate items in both slots
		int32 Slot = Items.Find(ItemId);
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green,
		                                 FString::Printf(TEXT("New item loaded (slot=%i): %s"), Slot, *Data->DisplayName.ToString()));
	}
	OnItemLoadedDelegate.Broadcast(ItemId, Data);
}
