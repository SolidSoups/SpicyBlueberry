// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZ_DeliveryTypes.generated.h"

class APZ_DeliveryPoint;

USTRUCT(BlueprintType)
struct FPZ_Order
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APZ_DeliveryPoint> Destination = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float IssuedTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 BaseReward = 100;

	UPROPERTY(BlueprintReadOnly)
	bool IsFulfilled = false;
};