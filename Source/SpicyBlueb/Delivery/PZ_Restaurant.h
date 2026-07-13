// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpicyBlueb/Interfaces/PZ_Interactable.h"
#include "PZ_Restaurant.generated.h"

class UPZ_ItemDataAsset;
class APZ_Pizza;
class APZ_PlayerCharacter;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class SPICYBLUEB_API APZ_Restaurant : public AActor, public IPZ_Interactable
{
public:

private:
	GENERATED_BODY()

public:
	APZ_Restaurant();
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restaurant")
	int32 OwningPlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restaurant")
	TSubclassOf<APZ_Pizza> PizzaClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restaurant")
	FPrimaryAssetId PizzaItemId;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);
	UFUNCTION()
	void OnZoneEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Restaurant")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Restaurant")
	TObjectPtr<UBoxComponent> PickupZone;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Restaurant")
	TObjectPtr<USceneComponent> CounterPoint;
	
	UPROPERTY()
	TObjectPtr<class UTextRenderComponent> TextComp;

private:
	bool IsOwnedBy(const APZ_PlayerCharacter* Char) const;
};