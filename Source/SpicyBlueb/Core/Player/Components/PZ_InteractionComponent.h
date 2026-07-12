// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_InteractionComponent.generated.h"


class APZ_ItemDummy;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZ_InteractionComponent();
	
	void AddInteractable(APZ_ItemDummy* Pickup);
	void RemoveInteractable(APZ_ItemDummy* Pickup);
	APZ_ItemDummy* GetClosestInteractable();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()	
	TObjectPtr<APZ_ItemDummy> OverlappingItemPickup;
	
	UPROPERTY()
	TArray<TObjectPtr<APZ_ItemDummy>> Interactables;
};
