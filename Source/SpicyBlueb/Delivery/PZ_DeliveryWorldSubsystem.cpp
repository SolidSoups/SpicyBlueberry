// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_DeliveryWorldSubsystem.h"
#include "PZ_DeliveryPoint.h"
#include "PZ_DeliverySettings.h"
#include "Algo/Find.h"
#include "Engine/Engine.h"
#include "SpicyBlueb/Delivery/PZ_DeliveryTypes.h"
#include "SpicyBlueb/PCG/PZ_CityGenerator.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "SpicyBlueb/Pizza/PZ_Pizza.h"
#include "Engine/World.h"

void UPZ_DeliveryWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize settings from ProjectSettings
	if (const UPZ_DeliverySettings* Settings = GetDefault<UPZ_DeliverySettings>())
	{
		MinOrdersPerBatch = Settings->MinOrdersPerBatch;
		MaxOrdersPerBatch = Settings->MaxOrdersPerBatch;
		ActivePointCount = Settings->ActivePointCount;
		MinRestaurantClearance = Settings->MinRestaurantClearance;
		FirstDeliveryBonus = Settings->FirstDeliveryBonus;
		MaxTimeBonus = Settings->MaxTimeBonus;
		TimeBonusWindow = Settings->TimeBonusWindow;
		DeliveryPointClass = Settings->DeliveryPointClass;
	}
}

void UPZ_DeliveryWorldSubsystem::StartCity(APZ_CityGenerator* CityGenerator, const TArray<FVector>& RestaurantLocations)
{
	City = CityGenerator;
	if (!City) return;

	FreeDeliveryCandidates = City->GetDeliveryCandidates();
	const float ClearSq = MinRestaurantClearance * MinRestaurantClearance;
	FreeDeliveryCandidates.RemoveAll([&](const FVector& C)
	{
		for (const FVector& R : RestaurantLocations)
		{
			if (FVector::DistSquared(C, R) < ClearSq)
			{
				return true;
			}
		}
		return false;
	});

	SpawnPoints();
}

TArray<FVector> UPZ_DeliveryWorldSubsystem::GetDeliveryLocations() const
{
	TArray<FVector> DeliveryLocations;
	for (const TObjectPtr<APZ_DeliveryPoint>& DeliveryPoint : ActiveDeliveryPoints)
	{
		if (DeliveryPoint)
		{
			DeliveryLocations.Add(DeliveryPoint->GetActorLocation());
		}
	}
	return DeliveryLocations;
}

void UPZ_DeliveryWorldSubsystem::SpawnPoints()
{
	if (!DeliveryPointClass || FreeDeliveryCandidates.Num() == 0) return;

	const int32 Count = FMath::Min(ActivePointCount, FreeDeliveryCandidates.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		// Spread picks across the candidate list.
		const int32 Idx = (i * FreeDeliveryCandidates.Num()) / Count;
		const FVector Loc = FreeDeliveryCandidates[Idx];

		FActorSpawnParameters Params;
		
		APZ_DeliveryPoint* P = GetWorld()->SpawnActor<APZ_DeliveryPoint>(
			DeliveryPointClass, Loc, FRotator::ZeroRotator, Params);
		if (P)
		{
			ActiveDeliveryPoints.Add(P);
			FreeDeliveryCandidates.RemoveAll([&](const FVector& C) { return C.Equals(Loc, 1.f); });
		}
	}
}

void UPZ_DeliveryWorldSubsystem::IssueBatch(APZ_PlayerState* Player) const
{
	if (!Player || ActiveDeliveryPoints.Num() == 0) return;

	Player->ActiveOrders.Reset();

	// A batch is simply N deliveries owed. The player chooses which active
	// point to deliver each one to - orders are not tied to a specific point.
	const int32 NumOrders = FMath::Clamp(
		FMath::RandRange(MinOrdersPerBatch, MaxOrdersPerBatch),
		1, MaxOrdersPerBatch);

	for (int32 i = 0; i < NumOrders; ++i)
	{
		FPZ_Order Order;
		Order.Destination = nullptr; // any active point satisfies this order
		Order.IssuedTime = GetWorld()->GetTimeSeconds();
		Order.BaseReward = 100;
		Player->ActiveOrders.Add(Order);
	}
}

int32 UPZ_DeliveryWorldSubsystem::TryDeliver(APZ_PlayerState* Player, APZ_DeliveryPoint* Point, APZ_Pizza* Pizza)
{
	if (!Player || !Point || !Pizza) return 0;

	// The point must be a currently active delivery point.
	if (!ActiveDeliveryPoints.Contains(Point))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Not an active delivery point"));
		return 0;
	}

	// Free choice: fulfill the first unfulfilled order in the batch,
	// regardless of which point it is. The player picks where to deliver.
	FPZ_Order* Match = Algo::FindByPredicate(Player->ActiveOrders, [](const FPZ_Order& Order){ return !Order.IsFulfilled; });	
	if (!Match)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("At point but no order routes here"));
		return 0; // no order for this point.
	}

	const int32 Reward = ComputeReward(*Match, Pizza, Point);
	Player->AddScore(Reward);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green,
		                                 FString::Printf(
			                                 TEXT("DELIVERED! +%d  (quality=%.0f)"), Reward, Pizza->Quality));

	Match->IsFulfilled = true;

	// First delivery bonus is consumed; mark the point claimed.
	const bool bWasFirst = !Point->bFirstDeliveryClaimed;
	if (bWasFirst)
	{
		Point->bFirstDeliveryClaimed = true;
	}

	Pizza->Destroy();

	// Stable then fresh: this point has now been cashed in, so it moves.
	RelocatePoint(Point);

	// If the player's whole route is done, hand them a new batch.
	bool bAllDone = true;
	for (const FPZ_Order& O : Player->ActiveOrders)
	{
		if (!O.IsFulfilled)
		{
			bAllDone = false;
			break;
		}
	}
	if (bAllDone)
	{
		IssueBatch(Player);
	}

	return Reward;
}

void UPZ_DeliveryWorldSubsystem::RelocatePoint(APZ_DeliveryPoint* Point)
{
	if (!Point) return;

	// Return the point's current spot to the free pool, take a fresh one.
	const FVector Old = Point->GetActorLocation();

	if (FreeDeliveryCandidates.Num() == 0)
	{
		// Nowhere fresh to go; just reset its first-delivery flag in place.
		Point->ResetForNewBatch();
		return;
	}

	const int32 Pick = FMath::RandRange(0, FreeDeliveryCandidates.Num() - 1);
	const FVector NewLoc = FreeDeliveryCandidates[Pick];
	FreeDeliveryCandidates.RemoveAt(Pick);
	FreeDeliveryCandidates.Add(Old);

	Point->Relocate(NewLoc);
	Point->ResetForNewBatch();
}

int32 UPZ_DeliveryWorldSubsystem::ComputeReward(const FPZ_Order& Order, APZ_Pizza* Pizza, APZ_DeliveryPoint* Point) const
{
	// Quality scales the base.
	const float QualityFrac = Pizza ? FMath::Clamp(Pizza->Quality / 100.f, 0.f, 1.f) : 1.f;
	int32 Reward = FMath::RoundToInt(Order.BaseReward * QualityFrac);

	// Time bonus decays linearly since the order was issued.
	const float Elapsed = GetWorld()->GetTimeSeconds() - Order.IssuedTime;
	const float TimeFrac = FMath::Clamp(1.f - (Elapsed / TimeBonusWindow), 0.f, 1.f);
	Reward += FMath::RoundToInt(MaxTimeBonus * TimeFrac);

	// First-to-this-point bonus.
	if (Point && !Point->bFirstDeliveryClaimed)
	{
		Reward += FirstDeliveryBonus;
	}

	return Reward;
}
