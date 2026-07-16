// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "PZ_InteractableVolumeComponent.generated.h"


UCLASS(ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_InteractableVolumeComponent : public UActorComponent
{

private:
	GENERATED_BODY()

public:
	UPZ_InteractableVolumeComponent();
	virtual void BeginPlay() override;
	void RegisterVolume(UPrimitiveComponent* InVolume);	
	
protected:
	UPROPERTY()	
	TObjectPtr<UPrimitiveComponent> PendingVolume;
	
	UFUNCTION()
	void OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};