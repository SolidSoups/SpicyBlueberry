// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_Shovel.generated.h"

UCLASS()
class SPICYBLUEB_API APZ_Shovel : public AActor
{
	GENERATED_BODY()

public:
	APZ_Shovel();
	
	
	UPROPERTY(VisibleAnywhere, Category = "Shovel")
	TObjectPtr<class UStaticMeshComponent> Handle;
	
	UPROPERTY(VisibleAnywhere, Category = "Shovel")
	TObjectPtr<class UStaticMeshComponent> Blade;
	
	UPROPERTY(VisibleAnywhere, Category = "Shovel|Combat")
	TObjectPtr<class UBoxComponent> HitVolume;
	
	void SetHitVolumeEnabled(bool IsEnabled);
	
	UPROPERTY(EditAnywhere, Category = "Shovel|Combat")
	float LaunchForce = 800.f;
	
	UPROPERTY(EditAnywhere, Category = "Shovel|Combat")
	float LaunchAngle = 35.f;
	
	FVector ComputeLaunchDirection(const FVector& Forward2D) const;	
private:
	
	UFUNCTION()
	void OnHitVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY()
	TSet<AActor*> HitActorsThisSwing;
};
