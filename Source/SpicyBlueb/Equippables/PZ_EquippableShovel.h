// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZ_EquippableActor.h"
#include "PZ_EquippableShovel.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class SPICYBLUEB_API APZ_EquippableShovel : public APZ_EquippableActor
{
	GENERATED_BODY()

public:
	APZ_EquippableShovel();
	virtual void Activate(APZ_PlayerCharacter* InstigatorOwner) override;	
	virtual void OnEquip(APZ_PlayerCharacter* NetOwner) override;
	virtual void OnUnequip() override;
	virtual void SetActiveVolumeEnabled(bool IsEnabled) override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Shovel")
	TObjectPtr<UStaticMeshComponent> Handle;
	
	UPROPERTY(VisibleAnywhere, Category = "Shovel")
	TObjectPtr<UStaticMeshComponent> Blade;
	
	UPROPERTY(VisibleAnywhere, Category = "Shovel|Combat")
	TObjectPtr<UBoxComponent> HitVolume;
	
	UPROPERTY(EditAnywhere, Category = "Shovel|Combat")
	float LaunchForce = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = "Shovel|Combat")
	float LaunchAngle = 35.f;
	
private:
	FVector ComputeLaunchDirection(const FVector& Forward2D) const;
	
	UFUNCTION()
	void OnHitVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY()
	TSet<AActor*> HitActorsThisSwing;
};
