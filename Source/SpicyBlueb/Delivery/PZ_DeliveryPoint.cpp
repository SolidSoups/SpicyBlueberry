// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_DeliveryPoint.h"
#include "PZ_DeliveryWorldSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "Engine/World.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "SpicyBlueb/Core/Player/Components/PZ_InteractionComponent.h"
#include "SpicyBlueb/Inventory/PZ_InventoryComponent.h"

APZ_DeliveryPoint::APZ_DeliveryPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	AcceptVolume = CreateDefaultSubobject<USphereComponent>(TEXT("AcceptVolume"));
	SetRootComponent(AcceptVolume);
	AcceptVolume->InitSphereRadius(AcceptRadius);
	AcceptVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AcceptVolume->SetCollisionObjectType(ECC_WorldDynamic);
	AcceptVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	AcceptVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AcceptVolume->SetGenerateOverlapEvents(true);

	Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	Marker->SetupAttachment(AcceptVolume);
	Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APZ_DeliveryPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APZ_DeliveryPoint, bFirstDeliveryClaimed);
}

void APZ_DeliveryPoint::OnInteract(APZ_PlayerCharacter* Interactor)
{
	if (!HasAuthority()) return;
	if (!IsValid(Interactor)) return;

	APZ_PlayerState* PS = Cast<APZ_PlayerState>(Interactor->GetPlayerState());
	if (!PS) return;
	
	auto* InteractionComp = Interactor->GetInteractionComponent();
	auto* InventoryComp = Interactor->GetInventoryComponent();
	if (!IsValid(InteractionComp) or !IsValid(InventoryComp)) return;
	
	if (InventoryComp->TryRemoveItem(PizzaItemId))
	{
		// Remove successful, player had that item
		
		// Hand off to delivery system
		//if (UPZ_DeliveryWorldSubsystem* DeliverySS = GetWorld()->GetSubsystem<UPZ_DeliveryWorldSubsystem>())
		//{
		//	if (DeliverySS->TryDeliver(PS, this, nullptr) > 0)
		//	{
		//     ClearCarriedPizza();
		//	}
		//}
	}
}

void APZ_DeliveryPoint::BeginPlay()
{
	Super::BeginPlay();
	AcceptVolume->SetSphereRadius(AcceptRadius);
	AcceptVolume->OnComponentBeginOverlap.AddDynamic(this, &APZ_DeliveryPoint::OnAcceptOverlap);
	AcceptVolume->OnComponentEndOverlap.AddDynamic(this, &APZ_DeliveryPoint::OnAcceptEndOverlap);
}

void APZ_DeliveryPoint::OnAcceptOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& Sweep)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!IsValid(PZCharacter)) return;

	auto* InteractionComp = PZCharacter->GetInteractionComponent();
	if (!InteractionComp) return;

	InteractionComp->AddInteractable(this);
}

void APZ_DeliveryPoint::OnAcceptEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!IsValid(PZCharacter)) return;

	auto* InteractionComp = PZCharacter->GetInteractionComponent();
	if (!InteractionComp) return;

	InteractionComp->RemoveInteractable(this);
}

void APZ_DeliveryPoint::Relocate(const FVector& NewLocation)
{
	if (!HasAuthority()) return;
	SetActorLocation(NewLocation);
}

void APZ_DeliveryPoint::ResetForNewBatch()
{
	if (!HasAuthority()) return;
	bFirstDeliveryClaimed = false;
}
