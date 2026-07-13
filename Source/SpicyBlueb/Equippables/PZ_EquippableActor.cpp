// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_EquippableActor.h"

#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"

APZ_EquippableActor::APZ_EquippableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APZ_EquippableActor::Server_Activate_Implementation()
{
	Activate(Cast<APZ_PlayerCharacter>(GetOwner()));
}
