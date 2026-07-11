// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZ_DeliveryPoint.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "PZ_DeliverySettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Pizza - Delivery Settings"))
class SPICYBLUEB_API UPZ_DeliverySettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Orders")
	int32 MinOrdersPerBatch = 2;

	UPROPERTY(Config, EditAnywhere, Category = "Orders")
	int32 MaxOrdersPerBatch = 4;

	UPROPERTY(Config, EditAnywhere, Category = "Match")
	int32 ActivePointCount = 4;

	UPROPERTY(Config, EditAnywhere, Category = "Match")
	float MinRestaurantClearance = 600.f;

	UPROPERTY(Config, EditAnywhere, Category = "Score")
	int32 FirstDeliveryBonus = 50;

	UPROPERTY(Config, EditAnywhere, Category = "Score")
	int32 MaxTimeBonus = 50;

	UPROPERTY(Config, EditAnywhere, Category = "Score")
	float TimeBonusWindow = 60.f;

	UPROPERTY(Config, EditAnywhere, Category = "Classes")
	TSubclassOf<APZ_DeliveryPoint> DeliveryPointClass;
};
