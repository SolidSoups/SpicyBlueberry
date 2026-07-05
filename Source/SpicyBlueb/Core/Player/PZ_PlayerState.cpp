// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_PlayerState.h"
#include "Net/UnrealNetwork.h"

APZ_PlayerState::APZ_PlayerState()
{
}

void APZ_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APZ_PlayerState, MatchScore);
	DOREPLIFETIME(APZ_PlayerState, ActiveOrders);
	DOREPLIFETIME(APZ_PlayerState, CurrentTask);
	DOREPLIFETIME(APZ_PlayerState, CurrentAbility);
}

void APZ_PlayerState::AddScore(int32 Delta)
{
	if (!HasAuthority()) return;
	MatchScore += Delta;
	OnRep_MatchScore();
}

void APZ_PlayerState::GrantAbility(EPZ_Ability NewAbility)
{
	if (!HasAuthority()) return;
	CurrentAbility = NewAbility;
}

void APZ_PlayerState::AssignTask(EPZ_BossTask NewTask)
{
	if (!HasAuthority()) return;
	CurrentTask = NewTask;
}

void APZ_PlayerState::OnRep_MatchScore()
{
	// TODO: broadcast a delegate for the HUD score widget.
}
