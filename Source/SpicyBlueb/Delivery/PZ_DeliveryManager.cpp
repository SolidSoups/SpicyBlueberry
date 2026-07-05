// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_DeliveryManager.h"
#include "PZ_DeliveryPoint.h"
#include "Engine/Engine.h"
#include "SpicyBlueb/Delivery/PZ_DeliveryTypes.h"
#include "SpicyBlueb/PCG/PZ_CityGenerator.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerState.h"
#include "SpicyBlueb/Pizza/PZ_Pizza.h"
#include "Engine/World.h"

APZ_DeliveryManager::APZ_DeliveryManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void APZ_DeliveryManager::Initialize(APZ_CityGenerator* InCity)
{
	if (!HasAuthority()) return;
	City = InCity;
	if (!City) return;
	
	FreeCandidates = City->GetDeliveryCandidates();

	SpawnPoints();
}

void APZ_DeliveryManager::SpawnPoints()
{
	if (!DeliveryPointClass || FreeCandidates.Num() == 0) return;

	const int32 Count = FMath::Min(ActivePointCount, FreeCandidates.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		// Spread picks across the candidate list.
		const int32 Idx = (i * FreeCandidates.Num()) / Count;
		const FVector Loc = FreeCandidates[Idx];

		FActorSpawnParameters Params;
		APZ_DeliveryPoint* P = GetWorld()->SpawnActor<APZ_DeliveryPoint>(
			DeliveryPointClass, Loc, FRotator::ZeroRotator, Params);
		if (P)
		{
			ActivePoints.Add(P);
			FreeCandidates.RemoveAll([&](const FVector& C) { return C.Equals(Loc, 1.f); });
		}
	}
}

void APZ_DeliveryManager::IssueBatch(APZ_PlayerState* Player)
{
	if (!HasAuthority() || !Player || ActivePoints.Num() == 0) return;

	Player->ActiveOrders.Reset();

	const int32 NumOrders = FMath::Clamp(
		FMath::RandRange(MinOrdersPerBatch, MaxOrdersPerBatch),
		1, ActivePoints.Num());

	// Distinct points per batch so a route has multiple stops.
	TArray<int32> Indices;
	for (int32 i = 0; i < ActivePoints.Num(); ++i) Indices.Add(i);
	for (int32 i = 0; i < NumOrders; ++i)
	{
		const int32 Pick = FMath::RandRange(i, Indices.Num() - 1);
		Indices.Swap(i, Pick);

		FPZ_Order Order;
		Order.Destination = ActivePoints[Indices[i]];
		Order.IssuedTime = GetWorld()->GetTimeSeconds();
		Order.BaseReward = 100;
		Player->ActiveOrders.Add(Order);
	}
}

int32 APZ_DeliveryManager::TryDeliver(APZ_PlayerState* Player, APZ_DeliveryPoint* Point, APZ_Pizza* Pizza)
{
	if (!HasAuthority() || !Player || !Point || !Pizza) return 0;

	// Find a matching, unfulfilled order routing to this point.
	FPZ_Order* Match = nullptr;
	for (FPZ_Order& O : Player->ActiveOrders)
	{
		if (!O.bFulfilled && O.Destination == Point)
		{
			Match = &O;
			break;
		}
	}
	
	if (!Match)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("At point but no order routes here"));
		return 0; // no order for this point.
	}

	const int32 Reward = ComputeReward(*Match, Pizza, Point);
	Player->AddScore(Reward);
	
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, FString::Printf(TEXT("DELIVERED! +%d  (quality=%.0f)"), Reward, Pizza->Quality));

	Match->bFulfilled = true;

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
		if (!O.bFulfilled)
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

void APZ_DeliveryManager::RelocatePoint(APZ_DeliveryPoint* Point)
{
	if (!Point) return;

	// Return the point's current spot to the free pool, take a fresh one.
	const FVector Old = Point->GetActorLocation();

	if (FreeCandidates.Num() == 0)
	{
		// Nowhere fresh to go; just reset its first-delivery flag in place.
		Point->ResetForNewBatch();
		return;
	}

	const int32 Pick = FMath::RandRange(0, FreeCandidates.Num() - 1);
	const FVector NewLoc = FreeCandidates[Pick];
	FreeCandidates.RemoveAt(Pick);
	FreeCandidates.Add(Old);

	Point->Relocate(NewLoc);
	Point->ResetForNewBatch();
}

int32 APZ_DeliveryManager::ComputeReward(const FPZ_Order& Order, APZ_Pizza* Pizza, APZ_DeliveryPoint* Point) const
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
