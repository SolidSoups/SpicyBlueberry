// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_PlayerController.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "Engine/Engine.h"
#include "SpicyBlueb/Core/GameState/PZ_GameState.h"

void APZ_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
}

void APZ_PlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (APZ_PlayerState* PS = GetPlayerState<APZ_PlayerState>())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, FString::Printf(TEXT("Score: %d   Orders: %d"), PS->MatchScore, PS->ActiveOrders.Num()));
	}
	
	if (APZ_GameState* GS = GetWorld()->GetGameState<APZ_GameState>())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::White, FString::Printf(TEXT("Time: %.0f   Phase: %d"),	GS->MatchTimeRemaining, (int32)GS->MatchPhase));
	}
}
