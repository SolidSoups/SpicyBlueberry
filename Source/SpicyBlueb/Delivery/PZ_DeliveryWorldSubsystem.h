// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "PZ_DeliveryWorldSubsystem.generated.h"

class APZ_CityGenerator;
class APZ_DeliveryPoint;
class APZ_PlayerState;
class APZ_Pizza;
struct FPZ_Order;

UCLASS()
class SPICYBLUEB_API UPZ_DeliveryWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void StartCity(APZ_CityGenerator* CityGenerator, const TArray<FVector>& RestaurantLocations);	
	
	void IssueBatch(APZ_PlayerState* Player) const;
	int32 TryDeliver(APZ_PlayerState* Player, APZ_DeliveryPoint* Point, APZ_Pizza* Pizza);
	const TArray<TObjectPtr<APZ_DeliveryPoint>>& GetActivePoints() const { return ActiveDeliveryPoints; }
	
	TArray<FVector> GetDeliveryLocations() const;

private:
	void SpawnPoints();
	void RelocatePoint(APZ_DeliveryPoint* Point);
	int32 ComputeReward(const FPZ_Order& Order, APZ_Pizza* Pizza, APZ_DeliveryPoint* Point) const;
	
	UPROPERTY()
	TObjectPtr<APZ_CityGenerator> City;

	UPROPERTY()
	TArray<TObjectPtr<APZ_DeliveryPoint>> ActiveDeliveryPoints;
	
	TArray<FVector> FreeDeliveryCandidates;
	
	// settings shit
	int32 MinOrdersPerBatch = 2;
	int32 MaxOrdersPerBatch = 4;
	int32 ActivePointCount = 4;
	float MinRestaurantClearance = 600.f;
	int32 FirstDeliveryBonus = 50;
	int32 MaxTimeBonus = 50;
	float TimeBonusWindow = 60.f;
	TSubclassOf<APZ_DeliveryPoint> DeliveryPointClass;
};