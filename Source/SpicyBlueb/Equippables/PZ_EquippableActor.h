// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_EquippableActor.generated.h"

class UAnimMontage;
class APZ_PlayerCharacter;

UCLASS(Abstract)
class SPICYBLUEB_API APZ_EquippableActor : public AActor
{
	GENERATED_BODY()

public:
	APZ_EquippableActor();
	
	UFUNCTION(Server, Reliable)
	void Server_Activate();
	
	virtual void Activate(APZ_PlayerCharacter* InstigatorOwner) {}	
	virtual void OnEquip(APZ_PlayerCharacter* NetOwner) {}
	virtual void SetActiveVolumeEnabled(bool IsEnabled) {}
	virtual void OnUnequip() {}
	
	UPROPERTY(EditDefaultsOnly, Category="Equippable")
	FName SocketName;
	
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	
};
