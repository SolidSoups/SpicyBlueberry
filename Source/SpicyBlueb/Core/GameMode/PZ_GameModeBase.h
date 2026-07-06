// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PZ_GameModeBase.generated.h"

class APZ_CityGenerator;
class APZ_Restaurant;
class APZ_PlayerState;
class APZ_GameState;
class APZ_DeliveryManager;

UCLASS()
class SPICYBLUEB_API APZ_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	APZ_GameModeBase();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MatchLengthSeconds = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<APZ_Restaurant> RestaurantClass;

private:
	UPROPERTY()
	TObjectPtr<APZ_CityGenerator> City;

	UPROPERTY()
	TArray<TObjectPtr<APZ_Restaurant>> Restaurants;

	FTimerHandle MatchTickTimer;

	void HookCity();
	void OnCityReady();
	void SpawnRestaurants();

	void StartMatch();
	void TickMatch();
	void EndMatch();

	APZ_GameState* GetPZGameState() const;
};
