// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_Shovel.h"

#include "Components/StaticMeshComponent.h"


APZ_Shovel::APZ_Shovel()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	Handle->SetupAttachment(RootComponent);
	
	Blade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Blade"));
	Blade->SetupAttachment(RootComponent);
}

