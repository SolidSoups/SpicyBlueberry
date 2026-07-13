// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpicyBlueb/Interfaces/PZ_Interactable.h"
#include "PZ_ItemDummy.generated.h"

class UPZ_ItemDataAsset;

/* A world actor which merely pretends to be an item. Stripped to its core, its only purpose is to display the model, do physics and be picked up. */
UCLASS()
class SPICYBLUEB_API APZ_ItemDummy : public AActor, public IPZ_Interactable
{

private:
	GENERATED_BODY()

public:
	APZ_ItemDummy();
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) override;
	
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
