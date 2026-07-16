// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InteractableVolumeComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Interactables/PZ_Interactable.h"
#include "SpicyBlueb/Core/Components/PZ_InteractionComponent.h"

UPZ_InteractableVolumeComponent::UPZ_InteractableVolumeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_InteractableVolumeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (PendingVolume)
	{
		PendingVolume->OnComponentBeginOverlap.AddDynamic(this, &UPZ_InteractableVolumeComponent::OnVolumeBeginOverlap);	
		PendingVolume->OnComponentEndOverlap.AddDynamic(this, &UPZ_InteractableVolumeComponent::OnVolumeEndOverlap);	
	}
}

void UPZ_InteractableVolumeComponent::RegisterVolume(UPrimitiveComponent* InVolume)
{
	PendingVolume = InVolume;
}

void UPZ_InteractableVolumeComponent::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetOwner()->HasAuthority()) return;
	
	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!PZCharacter) return;
	
	auto* InteractionComp = PZCharacter->GetInteractionComponent();
	if (!InteractionComp) return;
	
	InteractionComp->AddInteractable(GetOwner());
}

void UPZ_InteractableVolumeComponent::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetOwner()->HasAuthority()) return;
	
	auto* PZCharacter = Cast<APZ_PlayerCharacter>(OtherActor);
	if (!PZCharacter) return;
	
	auto* InteractionComp = PZCharacter->GetInteractionComponent();
	if (!InteractionComp) return;
	
	InteractionComp->RemoveInteractable(GetOwner());
}

