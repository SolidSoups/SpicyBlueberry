// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_DeliveryManager.generated.h"

class APZ_CityGenerator;
class APZ_DeliveryPoint;
class APZ_PlayerState;
class APZ_Pizza;
struct FPZ_Order;

UCLASS()
class SPICYBLUEB_API APZ_DeliveryManager : public AActor
{
	GENERATED_BODY()

public:
	APZ_DeliveryManager();
	
	void Initialize(APZ_CityGenerator* InCity);
	void IssueBatch(APZ_PlayerState* Player);
	
	int32 TryDeliver(APZ_PlayerState* Player, APZ_DeliveryPoint* Point, APZ_Pizza* Pizza);

	const TArray<TObjectPtr<APZ_DeliveryPoint>>& GetActivePoints() const { return ActivePoints; }

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Orders")
	int32 MinOrdersPerBatch = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Orders")
	int32 MaxOrdersPerBatch = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 ActivePointCount = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 FirstDeliveryBonus = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 MaxTimeBonus = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	float TimeBonusWindow = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APZ_DeliveryPoint> DeliveryPointClass;

private:
	UPROPERTY()
	TObjectPtr<APZ_CityGenerator> City;

	UPROPERTY()
	TArray<TObjectPtr<APZ_DeliveryPoint>> ActivePoints;
	
	TArray<FVector> FreeCandidates;

	void SpawnPoints();
	void RelocatePoint(APZ_DeliveryPoint* Point);

	int32 ComputeReward(const FPZ_Order& Order, APZ_Pizza* Pizza, APZ_DeliveryPoint* Point) const;
};