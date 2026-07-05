// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PZ_GameState.generated.h"

UENUM(BlueprintType)
enum class EPZ_MatchPhase : uint8
{
	WaitingToStart UMETA(DisplayName = "Waiting To Start"),
	InProgress UMETA(DisplayName = "In Progress"),
	Finished UMETA(DisplayName = "Finished"),
	// Match phases could be added for mini games?
};

UCLASS()
class SPICYBLUEB_API APZ_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	APZ_GameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_TimeRemaining, BlueprintReadOnly, Category = "Match")
	float MatchTimeRemaining = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	EPZ_MatchPhase MatchPhase = EPZ_MatchPhase::WaitingToStart;

	UFUNCTION()
	void OnRep_TimeRemaining();
};
