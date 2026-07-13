// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_EquipmentComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Equippables/PZ_EquippableActor.h"
#include "SpicyBlueb/Inventory/PZ_InventoryComponent.h"
#include "SpicyBlueb/Inventory/PZ_ItemData.h"


UPZ_EquipmentComponent::UPZ_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPZ_EquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPZ_EquipmentComponent, CurrentEquipped);
}

void UPZ_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetOwner()->HasAuthority()) return;
	
	InventoryComponent = GetOwner()->GetComponentByClass<UPZ_InventoryComponent>();
	if (!InventoryComponent) return;
	
	InventoryComponent->OnSlotSelectedDelegate.AddLambda([this](int32)
	{
		RefreshEquippedItemFromSlot();
	});
	InventoryComponent->OnItemLoadedDelegate.AddLambda([this](int32, const UPZ_ItemDataAsset*)
	{
		RefreshEquippedItemFromSlot();
	});
	InventoryComponent->OnItemRemovedDelegate.AddLambda([this](int32)
	{
		RefreshEquippedItemFromSlot();
	});
}

void UPZ_EquipmentComponent::RefreshEquippedItemFromSlot()
{
	// SelectedData could be null if the inventory slot is empty. This is used to clear 
	// the equipment components state
	UPZ_ItemDataAsset* SelectedData = InventoryComponent->GetSelectedItemData();
	if (SelectedData == CurrentEquippedItemData) return;
	
	EquipItem(SelectedData);
}

void UPZ_EquipmentComponent::EquipItem(UPZ_ItemDataAsset* ItemData)
{
	if (CurrentEquipped)
	{
		CurrentEquipped->OnUnequip();
		CurrentEquipped->Destroy();
		CurrentEquipped = nullptr;
	}
	CurrentEquippedItemData = ItemData;
	
	if (!ItemData or ItemData->EquippableActorClass.IsNull()) return;
	
	// TODO [Elias Brown]: LoadSynchronous could cause a small hitch for the user when switching between slots
	TSubclassOf<APZ_EquippableActor> EquipClass = ItemData->EquippableActorClass.LoadSynchronous();
	if (!EquipClass) return;
	
	USkeletalMeshComponent* CharacterMesh = GetOwnerSkeletalMesh();
	if (!CharacterMesh) return;
	
	CurrentEquipped = GetWorld()->SpawnActor<APZ_EquippableActor>(EquipClass);
	CurrentEquipped->SetOwner(GetOwner());
	CurrentEquipped->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentEquipped->SocketName);
	CurrentEquipped->OnEquip(Cast<APZ_PlayerCharacter>(GetOwner()));
}

USkeletalMeshComponent* UPZ_EquipmentComponent::GetOwnerSkeletalMesh() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character ? Character->GetMesh() : nullptr;
}
