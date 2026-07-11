// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_ItemDummy.generated.h"

class UPZ_ItemDataAsset;
/* An item dummy is a useless little thing displayed in the world. The user
 * can pick it up at their leisure but it isn't supposed to do any actions. */
UCLASS()
class SPICYBLUEB_API APZ_ItemDummy : public AActor
{

private:
	GENERATED_BODY()

public:
	APZ_ItemDummy();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FPrimaryAssetId ItemId;
	
protected:
	UFUNCTION()
	void OnPickupVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
									const FHitResult& SweepResult);
	UFUNCTION()
	void OnPickupVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> PickupVolume;
	
};
