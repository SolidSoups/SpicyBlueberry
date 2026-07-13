// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_InteractionComponent.generated.h"


class IPZ_Interactable;
class APZ_ItemDummy;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPZ_InteractionComponent();
	void AddInteractable(const TScriptInterface<IPZ_Interactable>& Interactable);
	void RemoveInteractable(const TScriptInterface<IPZ_Interactable>& Interactable);
	TScriptInterface<IPZ_Interactable> GetClosestInteractable();

protected:
	UPROPERTY()
	TArray<TScriptInterface<IPZ_Interactable>> Interactables;
};
