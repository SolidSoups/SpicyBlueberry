// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_PlayerCharacter.h"


// Sets default values
APZ_PlayerCharacter::APZ_PlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APZ_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APZ_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APZ_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

