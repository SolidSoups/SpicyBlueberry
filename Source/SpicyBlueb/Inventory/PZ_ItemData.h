// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PZ_ItemData.generated.h"

/**
 * 
 */
UCLASS()
class SPICYBLUEB_API UPZ_ItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSoftClassPtr<class APZ_ItemDummy> DummyActorClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<class UTexture2D> Icon;
};
