// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SpicyBlueb/Delivery/PZ_DeliveryTypes.h"
#include "PZ_PlayerState.generated.h"

UENUM(BlueprintType)
enum class EPZ_BossTask : uint8
{
	None UMETA(DisplayName = "None"),
	PunchRunner UMETA(DisplayName = "Punch Algo Pizza Runner"),
	PickUpCheese UMETA(DisplayName = "Pick Up Cheese"),
	// Add more boss task that can come
};

UENUM(BlueprintType)
enum class EPZ_Ability : uint8
{
	None UMETA(DisplayName = "None"),
	// Add abilities from playerstate
};

UCLASS()
class SPICYBLUEB_API APZ_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	APZ_PlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_MatchScore, BlueprintReadOnly, Category = "Score")
	int32 MatchScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Orders")
	TArray<FPZ_Order> ActiveOrders;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss")
	EPZ_BossTask CurrentTask = EPZ_BossTask::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ability")
	EPZ_Ability CurrentAbility = EPZ_Ability::None;

	// Add to the score.
	void AddScore(int32 Delta);

	void GrantAbility(EPZ_Ability NewAbility);
	void AssignTask(EPZ_BossTask NewTask);

protected:
	UFUNCTION()
	void OnRep_MatchScore();
};
