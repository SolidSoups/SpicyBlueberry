// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_ItemDummy.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"


APZ_ItemDummy::APZ_ItemDummy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(20.f));
	CollisionBox->SetCollisionProfileName(TEXT("PhysicsActor"));
	CollisionBox->SetSimulatePhysics(true);
	
	PickupVolume = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup Volume"));
	PickupVolume->SetupAttachment(CollisionBox);
	PickupVolume->SetSphereRadius(50.f);
	PickupVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}
