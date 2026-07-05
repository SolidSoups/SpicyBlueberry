// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_GameState.h"
#include "Net/UnrealNetwork.h"

APZ_GameState::APZ_GameState()
{
}

void APZ_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APZ_GameState, MatchTimeRemaining);
	DOREPLIFETIME(APZ_GameState, MatchPhase);
}

void APZ_GameState::OnRep_TimeRemaining()
{
	// TODO: broadcast to the HUD clock widget.
}
