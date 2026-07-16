// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpicyBlueb/Interactables/PZ_Interactable.h"
#include "PZ_DeliveryPoint.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class SPICYBLUEB_API APZ_DeliveryPoint : public AActor, public IPZ_Interactable
{

private:
	GENERATED_BODY()

public:
	APZ_DeliveryPoint();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Delivery")
	bool bFirstDeliveryClaimed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delivery")
	float AcceptRadius = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void Relocate(const FVector& NewLocation);
	
	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void ResetForNewBatch();

	USphereComponent* GetAcceptVolume() const { return AcceptVolume; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAcceptOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnAcceptEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery")
	TObjectPtr<USphereComponent> AcceptVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery")
	TObjectPtr<UStaticMeshComponent> Marker;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delivery")
	FPrimaryAssetId PizzaItemId;
};