// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InteractionComponent.h"

#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Inventory/PZ_ItemDummy.h"


UPZ_InteractionComponent::UPZ_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_InteractionComponent::AddInteractable(APZ_ItemDummy* Pickup)
{
	Interactables.Add(Pickup);
}

void UPZ_InteractionComponent::RemoveInteractable(APZ_ItemDummy* Pickup)
{
	if (!Interactables.Contains(Pickup)) return;
	Interactables.Remove(Pickup);
}

APZ_ItemDummy* UPZ_InteractionComponent::GetClosestInteractable()
{
	if (Interactables.IsEmpty()) return nullptr;	
	if (Interactables.Num() == 1) return Interactables[0];
	
	ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner());	
	if (!PlayerCharacter) return nullptr;
	
	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();	
	APZ_ItemDummy* BestInteractable = nullptr;
	float BestDistanceSquared = FLT_MAX;
	for (int32 i = 0; i < Interactables.Num(); i++)
	{
		if (!Interactables.IsValidIndex(i) or !IsValid(Interactables[i]))
			continue;
		
		const FVector InteractableLocation = Interactables[i]->GetActorLocation();		
		const float DistanceSquared = (InteractableLocation - PlayerLocation).SizeSquared();			
		if (DistanceSquared < BestDistanceSquared)
		{
			BestInteractable = Interactables[i];
			BestDistanceSquared = DistanceSquared;
		}
	}
	
	return BestInteractable;
}


void UPZ_InteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


