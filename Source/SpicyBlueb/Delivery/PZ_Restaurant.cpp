// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_Restaurant.h"
#include "SpicyBlueb/Pizza/PZ_Pizza.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "SpicyBlueb/Core/Player/Components/PZ_InteractionComponent.h"
#include "SpicyBlueb/Inventory/PZ_InventoryComponent.h"

APZ_Restaurant::APZ_Restaurant()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RestaurantMesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PickupZone = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupZone"));
	PickupZone->SetupAttachment(Mesh);
	PickupZone->SetBoxExtent(FVector(200.f, 200.f, 150.f));
	PickupZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupZone->SetCollisionObjectType(ECC_WorldDynamic);
	PickupZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupZone->SetGenerateOverlapEvents(true);

	CounterPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CounterPoint"));
	CounterPoint->SetupAttachment(Mesh);
	CounterPoint->SetRelativeLocation(FVector(150.f, 0.f, 100.f));

	// Create a text component too clearly see restaurant location
	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	TextComp->SetupAttachment(RootComponent);
	TextComp->SetText(FText::FromString("Sir, this is a restaurant."));
	TextComp->SetHorizontalAlignment(EHTA_Center);
	const FRotator TopDownBackward(55.f, 225.f, 0.f); // TODO: make this automatically become opposite camera rotation
	TextComp->SetWorldRotation(TopDownBackward);
	TextComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	TextComp->SetWorldSize(48.f);
}

void APZ_Restaurant::OnInteract(APZ_PlayerCharacter* Interactor)
{
	if (!HasAuthority()) return;
	if (!IsValid(Interactor)) return;
	if (!IsOwnedBy(Interactor)) return;
	if (!PizzaItemId.IsValid()) return;
	
	auto* InventoryComp = Interactor->GetInventoryComponent();
	auto* InteractionComp = Interactor->GetInteractionComponent();
	if (!IsValid(InventoryComp) or !IsValid(InteractionComp))
		return;
	
	// one piece at a time lil bro
	if (InventoryComp->HasItem(PizzaItemId))
		return;
	
	InteractionComp->RemoveInteractable(this);		
	InventoryComp->AddItem(PizzaItemId);
}

void APZ_Restaurant::BeginPlay()
{
	Super::BeginPlay();
	PickupZone->OnComponentBeginOverlap.AddDynamic(this, &APZ_Restaurant::OnZoneBeginOverlap);
	PickupZone->OnComponentEndOverlap.AddDynamic(this, &APZ_Restaurant::OnZoneEndOverlap);

	// Catch players already standing in the zone at spawn (server only).
	if (HasAuthority())
	{
		TArray<AActor*> Overlapping;
		PickupZone->GetOverlappingActors(Overlapping, APZ_PlayerCharacter::StaticClass());
		for (AActor* A : Overlapping)
		{
			if (APZ_PlayerCharacter* Char = Cast<APZ_PlayerCharacter>(A))
			{
				if (IsOwnedBy(Char))
				{
					Char->SetOverlappingRestaurant(this);
				}
			}
		}
	}
}

void APZ_Restaurant::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& Sweep)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!IsValid(PZCharacter) or !IsOwnedBy(PZCharacter)) return;

	auto* InteractionComp = PZCharacter->GetComponentByClass<UPZ_InteractionComponent>();
	if (!InteractionComp) return;

	InteractionComp->AddInteractable(this);
}

void APZ_Restaurant::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!IsValid(PZCharacter) or !IsOwnedBy(PZCharacter)) return;

	auto* InteractionComp = PZCharacter->GetComponentByClass<UPZ_InteractionComponent>();
	if (!InteractionComp) return;

	InteractionComp->RemoveInteractable(this);
}

bool APZ_Restaurant::IsOwnedBy(const APZ_PlayerCharacter* Char) const
{
	if (!Char) return false;

	if (const APlayerController* PC = Cast<APlayerController>(Char->GetController()))
	{
		if (const APlayerState* PS = PC->PlayerState)
		{
			return PS->GetPlayerId() == OwningPlayerIndex;
		}
	}
	return false;
}
