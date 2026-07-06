// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_DeliveryPoint.h"
#include "PZ_DeliveryWorldSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "Engine/World.h"

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

void APZ_DeliveryPoint::BeginPlay()
{
	Super::BeginPlay();
	AcceptVolume->SetSphereRadius(AcceptRadius);
	AcceptVolume->OnComponentBeginOverlap.AddDynamic(this, &APZ_DeliveryPoint::OnAcceptOverlap);
	AcceptVolume->OnComponentEndOverlap.AddDynamic(this, &APZ_DeliveryPoint::OnAcceptEndOverlap);
}

void APZ_DeliveryPoint::OnAcceptOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	// Server owns the overlap state that Interact() reads.
	if (!HasAuthority() || !OtherActor) return;

	if (APZ_PlayerCharacter* Char = Cast<APZ_PlayerCharacter>(OtherActor))
	{
		// Register this point so the player can deliver via Interact.
		Char->SetOverlappingDeliveryPoint(this);
	}
}

void APZ_DeliveryPoint::OnAcceptEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !OtherActor) return;

	if (APZ_PlayerCharacter* Char = Cast<APZ_PlayerCharacter>(OtherActor))
	{
		Char->ClearOverlappingDeliveryPoint(this);
	}
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