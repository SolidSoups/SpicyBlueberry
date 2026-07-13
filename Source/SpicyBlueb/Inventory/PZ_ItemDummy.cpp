// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_ItemDummy.h"

#include "PZ_InventoryComponent.h"
#include "PZ_ItemData.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Core/Player/Components/PZ_InteractionComponent.h"


APZ_ItemDummy::APZ_ItemDummy()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(20.f));
	CollisionBox->SetCollisionProfileName(TEXT("PhysicsActor"));
	CollisionBox->SetSimulatePhysics(true);

	PickupVolume = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup Volume"));
	PickupVolume->SetupAttachment(CollisionBox);
	PickupVolume->SetSphereRadius(50.f);
	PickupVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PickupVolume->OnComponentBeginOverlap.AddDynamic(this, &APZ_ItemDummy::OnPickupVolumeBeginOverlap);
	PickupVolume->OnComponentEndOverlap.AddDynamic(this, &APZ_ItemDummy::OnPickupVolumeEndOverlap);
}

void APZ_ItemDummy::OnInteract(APZ_PlayerCharacter* Interactor)
{
	if (!HasAuthority()) return;
	
	UPZ_InventoryComponent* InventoryComp = Interactor->GetComponentByClass<UPZ_InventoryComponent>();
	UPZ_InteractionComponent* InteractionComp = Interactor->GetComponentByClass<UPZ_InteractionComponent>();
	if (!IsValid(InventoryComp) or !IsValid(InteractionComp)) return;
	
	if (InventoryComp->AddItem(ItemId))
	{
		InteractionComp->RemoveInteractable(this);			
		Destroy();
		return;
	}	
}

void APZ_ItemDummy::OnPickupVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!PZCharacter) return;

	auto* InteractionComp = PZCharacter->GetComponentByClass<UPZ_InteractionComponent>();
	if (!InteractionComp) return;

	InteractionComp->AddInteractable(this);
}

void APZ_ItemDummy::OnPickupVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!PZCharacter) return;

	auto* InteractionComp = PZCharacter->GetComponentByClass<UPZ_InteractionComponent>();
	if (!InteractionComp) return;

	InteractionComp->RemoveInteractable(this);
}
