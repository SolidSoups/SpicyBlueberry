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

};
