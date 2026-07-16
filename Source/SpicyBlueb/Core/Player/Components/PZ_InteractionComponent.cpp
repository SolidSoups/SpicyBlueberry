// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InteractionComponent.h"

#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Inventory/PZ_ItemDummy.h"


UPZ_InteractionComponent::UPZ_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_InteractionComponent::AddInteractable(const TScriptInterface<IPZ_Interactable>& Interactable)
{
	Interactables.AddUnique(Interactable);
	Interactable->OnInteractZoneEntered(Cast<APZ_PlayerCharacter>(GetOwner()));
}

void UPZ_InteractionComponent::RemoveInteractable(const TScriptInterface<IPZ_Interactable>& Interactable)
{
	if (!Interactables.Contains(Interactable)) return;
	Interactables.Remove(Interactable);
	Interactable->OnInteractZoneExited(Cast<APZ_PlayerCharacter>(GetOwner()));
}

TScriptInterface<IPZ_Interactable> UPZ_InteractionComponent::GetClosestInteractable()
{
	if (Interactables.IsEmpty()) return nullptr;
	
	ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwner());	
	if (!PlayerCharacter) return nullptr;
	
	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	TScriptInterface<IPZ_Interactable> BestInteractable = nullptr;
	float BestDistanceSquared = FLT_MAX;
	
	for (const TScriptInterface<IPZ_Interactable>& Interactable : Interactables)
	{
		AActor* Actor = Cast<AActor>(Interactable.GetObject());	
		if (!IsValid(Actor)) continue;
		
		const float DistanceSquared = (Actor->GetActorLocation() - PlayerLocation).SizeSquared();	
		if (DistanceSquared < BestDistanceSquared)
		{
			BestInteractable = Interactable;
			BestDistanceSquared = DistanceSquared;
		}
	}
	
	return BestInteractable;
}