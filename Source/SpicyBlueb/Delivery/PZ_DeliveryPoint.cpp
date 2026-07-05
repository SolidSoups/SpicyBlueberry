// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_DeliveryPoint.h"
#include "PZ_DeliveryManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SpicyBlueb/Pizza/PZ_Pizza.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "SpicyBlueb/Core/GameMode/PZ_GameModeBase.h"
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
	AcceptVolume->SetCollisionResponseToAllChannels(ECR_Overlap);

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
}

void APZ_DeliveryPoint::OnAcceptOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority() || !OtherActor) return;

	APZ_PlayerCharacter* Char = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!Char) return;

	APZ_Pizza* Pizza = Char->GetCarriedPizza();
	if (!Pizza) return;

	APZ_PlayerState* PS = Char->GetPlayerState<APZ_PlayerState>();
	if (!PS) return;

	if (APZ_GameModeBase* GM = GetWorld()->GetAuthGameMode<APZ_GameModeBase>())
	{
		if (APZ_DeliveryManager* Mgr = GM->GetDeliveryManager())
		{
			const int32 Awarded = Mgr->TryDeliver(PS, this, Pizza);
			if (Awarded > 0)
			{
				Char->ClearCarriedPizza();
			}
		}
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