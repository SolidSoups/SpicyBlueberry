// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_GameModeBase.h"
#include "SpicyBlueb/Core/GameState/PZ_GameState.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "SpicyBlueb/PCG/PZ_CityGenerator.h"
#include "SpicyBlueb/Delivery/PZ_DeliveryWorldSubsystem.h"
#include "SpicyBlueb/Delivery/PZ_Restaurant.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"

APZ_GameModeBase::APZ_GameModeBase()
{
	GameStateClass = APZ_GameState::StaticClass();
	PlayerStateClass = APZ_PlayerState::StaticClass();
}

void APZ_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	//UGameplayStatics::CreatePlayer(this, 1, true);
	HookCity();
}

APZ_GameState* APZ_GameModeBase::GetPZGameState() const
{
	return GetGameState<APZ_GameState>();
}

void APZ_GameModeBase::HookCity()
{
	for (TActorIterator<APZ_CityGenerator> It(GetWorld()); It; ++It)
	{
		City = *It;
		break;
	}

	if (!City)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PZ] No PZ_CityGenerator in level; match cannot start."));
		return;
	}

	if (City->IsCityReady)
	{
		OnCityReady();
	}
	else
	{
		City->OnCityGenerated.AddUObject(this, &APZ_GameModeBase::OnCityReady);
	}
}

void APZ_GameModeBase::OnCityReady()
{
	SpawnRestaurants();

	// Spawn and initialize the delivery subsystem.
	if (UPZ_DeliveryWorldSubsystem* DeliverySS = GetWorld()->GetSubsystem<UPZ_DeliveryWorldSubsystem>())
	{
		// Gather restaurant spots so delivery points don't spawn on them.
		TArray<FVector> RestaurantLocations;
		RestaurantLocations.Reserve(Restaurants.Num());
		for (const TObjectPtr<APZ_Restaurant>& R : Restaurants)
		{
			if (R)
			{
				RestaurantLocations.Add(R->GetActorLocation());
			}
		}

		DeliverySS->StartCity(City, RestaurantLocations);
	}

	StartMatch();
}

void APZ_GameModeBase::SpawnRestaurants()
{
	if (!City || !RestaurantClass) return;

	APZ_GameState* GS = GetPZGameState();
	const int32 NumPlayers = FMath::Max(1, GetNumPlayers());
	const TArray<FVector> Spots = City->GetRestaurantSpawns(NumPlayers);

	for (int32 i = 0; i < Spots.Num(); ++i)
	{
		APZ_Restaurant* R = GetWorld()->SpawnActor<APZ_Restaurant>(RestaurantClass, Spots[i], FRotator::ZeroRotator);
		if (R)
		{
			if (GS && GS->PlayerArray.IsValidIndex(i))
			{
				R->OwningPlayerIndex = GS->PlayerArray[i]->GetPlayerId();
			}
			else
			{
				R->OwningPlayerIndex = i;
			}
			Restaurants.Add(R);
		}
	}
}

void APZ_GameModeBase::StartMatch()
{
	APZ_GameState* GS = GetPZGameState();
	if (!GS) return;
	
	UPZ_DeliveryWorldSubsystem* DeliverySS = GetWorld()->GetSubsystem<UPZ_DeliveryWorldSubsystem>();
	if (!DeliverySS) return;

	GS->MatchTimeRemaining = MatchLengthSeconds;
	GS->MatchPhase = EPZ_MatchPhase::InProgress;

	// Starting batch for each player.
	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APZ_PlayerState* PZPS = Cast<APZ_PlayerState>(PS))
		{
			DeliverySS->IssueBatch(PZPS);
		}
	}

	GetWorldTimerManager().SetTimer(MatchTickTimer, this, &APZ_GameModeBase::TickMatch, 1.0f, true);
}

void APZ_GameModeBase::TickMatch()
{
	APZ_GameState* GS = GetPZGameState();
	if (!GS) return;

	GS->MatchTimeRemaining = FMath::Max(0.f, GS->MatchTimeRemaining - 1.0f);
	GS->OnRep_TimeRemaining();

	if (GS->MatchTimeRemaining <= 0.f)
	{
		EndMatch();
	}
}

void APZ_GameModeBase::EndMatch()
{
	GetWorldTimerManager().ClearTimer(MatchTickTimer);

	if (APZ_GameState* GS = GetPZGameState())
	{
		GS->MatchPhase = EPZ_MatchPhase::Finished;
	}
	// TODO: winner = highest MatchScore across PlayerArray.
}
