// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventoryComponent.h"

#include "PZ_InventorySettings.h"
#include "PZ_ItemData.h"
#include "TimerManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UPZ_InventoryComponent::UPZ_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPZ_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UPZ_InventoryComponent, Items, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPZ_InventoryComponent, SelectedSlot, COND_OwnerOnly);
}

void UPZ_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const auto* InventorySettings = GetDefault<UPZ_InventorySettings>())
		MaxItemSlots = InventorySettings->MaxInventorySlots;
	else
		UE_LOG(LogTemp, Error, TEXT("Could not get inventory settings"));

	Items.SetNum(MaxItemSlots);
	ResolvedItemsData.SetNum(MaxItemSlots);
	PreviousAssetIds.SetNum(MaxItemSlots);
	IsSlotLoading.Add(false, MaxItemSlots);

	// Broadcast item selection so UI defaults to zero index (next tick so UI can catch up and bind)
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		OnSlotSelectedDelegate.Broadcast(0);
	});
}

void UPZ_InventoryComponent::SetSelectedSlot(int32 Slot)
{
	if (!Items.IsValidIndex(Slot))
	{
		UE_LOG(LogTemp, Error, TEXT("Slot out of range"));
		return;
	}

	SelectedSlot = Slot;
	MARK_PROPERTY_DIRTY_FROM_NAME(UPZ_InventoryComponent, SelectedSlot, this);
	OnSlotSelectedDelegate.Broadcast(Slot);
}

UPZ_ItemDataAsset* UPZ_InventoryComponent::GetItemData(int32 Slot) const
{
	if (!Items.IsValidIndex(Slot) or !Items[Slot].AssetId.IsValid() or IsSlotLoading[Slot])
		return nullptr;
	return ResolvedItemsData[Slot];
}

UPZ_ItemDataAsset* UPZ_InventoryComponent::GetSelectedItemData() const
{
	if (!Items.IsValidIndex(SelectedSlot) or IsSlotLoading[SelectedSlot])
		return nullptr;

	return ResolvedItemsData[SelectedSlot];
}

bool UPZ_InventoryComponent::HasItem(FPrimaryAssetId ItemId) const
{
	return FindSlotWithItem(ItemId) != INDEX_NONE;
}

int32 UPZ_InventoryComponent::FindSlotWithItem(FPrimaryAssetId ItemId) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (!IsSlotLoading[i] and Items[i].AssetId == ItemId)
			return i;
	}
	return INDEX_NONE;
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
		if (!Items.IsValidIndex(i) or IsSlotLoading[i] or Items[i].AssetId.IsValid())
			continue;

		FirstSlot = i;
		break;
	}
	if (FirstSlot <= -1)
		return false;

	IsSlotLoading[FirstSlot] = true;
	UAssetManager::Get().LoadPrimaryAsset(
		ItemId,
		TArray<FName>(),
		FStreamableDelegate::CreateUObject(this, &UPZ_InventoryComponent::OnItemLoaded, FirstSlot, ItemId));

	return true;
}

bool UPZ_InventoryComponent::TryRemoveItem(FPrimaryAssetId ItemId)
{
	int32 FoundSlot = FindSlotWithItem(ItemId);
	if (FoundSlot == INDEX_NONE) return false;

	DeleteSlot(FoundSlot);
	OnItemRemovedDelegate.Broadcast(FoundSlot);
	return true;
}

FPrimaryAssetId UPZ_InventoryComponent::TryPopItemSlot(int32 Slot)
{
	if (!Items.IsValidIndex(Slot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove an item from an unused slot (%i)"), Slot);
		return FPrimaryAssetId{};
	}

	if (IsSlotLoading[Slot])
		return FPrimaryAssetId{};

	if (!Items[Slot].AssetId.IsValid())
		return FPrimaryAssetId{};

	FPrimaryAssetId AssetId = Items[Slot].AssetId;
	DeleteSlot(Slot);
	OnItemRemovedDelegate.Broadcast(Slot);

	return AssetId;
}

void UPZ_InventoryComponent::OnItemLoaded(const int32 Slot, const FPrimaryAssetId AssetId)
{
	if (!IsSlotLoading[Slot]) return;

	UPZ_ItemDataAsset* ItemData = Cast<UPZ_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(AssetId));

	// occupy the slot
	Items[Slot].AssetId = AssetId;
	PreviousAssetIds[Slot] = AssetId;
	ResolvedItemsData[Slot] = ItemData;
	IsSlotLoading[Slot] = false;

	if (GetOwner()->HasAuthority())
		MARK_PROPERTY_DIRTY_FROM_NAME(UPZ_InventoryComponent, Items, this);

	OnItemLoadedDelegate.Broadcast(Slot, ItemData);
}

void UPZ_InventoryComponent::DeleteSlot(int32 Slot)
{
	if (!Items.IsValidIndex(Slot))
		return;

	if (IsSlotLoading[Slot] or !Items[Slot].AssetId.IsValid())
		return;

	UAssetManager::Get().UnloadPrimaryAsset(Items[Slot].AssetId);

	Items[Slot].AssetId = FPrimaryAssetId{};
	ResolvedItemsData[Slot] = nullptr;
	PreviousAssetIds[Slot] = FPrimaryAssetId{};
	IsSlotLoading[Slot] = false;

	if (GetOwner()->HasAuthority())
		MARK_PROPERTY_DIRTY_FROM_NAME(UPZ_InventoryComponent, Items, this);
}

void UPZ_InventoryComponent::OnRep_Items()
{
	// Check the difference between our previous asset ids and the new replicated ones
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (!PreviousAssetIds.IsValidIndex(i) or Items[i].AssetId == PreviousAssetIds[i])
			continue;

		const FPrimaryAssetId OldAssetId = PreviousAssetIds[i];
		PreviousAssetIds[i] = Items[i].AssetId;

		if (OldAssetId.IsValid())
		{
			UAssetManager::Get().UnloadPrimaryAsset(OldAssetId);
			ResolvedItemsData[i] = nullptr;
			OnItemRemovedDelegate.Broadcast(i);
		}

		if (Items[i].AssetId.IsValid())
		{
			IsSlotLoading[i] = true;
			UAssetManager::Get().LoadPrimaryAsset(
				Items[i].AssetId,
				TArray<FName>(),
				FStreamableDelegate::CreateUObject(this, &UPZ_InventoryComponent::OnItemLoaded, i, Items[i].AssetId)
			);
		}
	}
}

void UPZ_InventoryComponent::OnRep_SelectedSlot()
{
	OnSlotSelectedDelegate.Broadcast(SelectedSlot);
}
