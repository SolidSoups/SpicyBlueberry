// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_ItemDummy.generated.h"

/* An item dummy is a useless little thing displayed in the world. The user
 * can pick it up at their leisure but it isn't supposed to do any actions. */
UCLASS()
class SPICYBLUEB_API APZ_ItemDummy : public AActor
{
	GENERATED_BODY()

public:
	APZ_ItemDummy();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FPrimaryAssetId ItemId;
	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> PickupVolume;
};
