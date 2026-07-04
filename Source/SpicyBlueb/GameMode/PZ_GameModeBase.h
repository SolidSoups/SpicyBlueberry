// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PZ_GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SPICYBLUEB_API APZ_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APZ_GameModeBase();
	
protected:
	virtual void BeginPlay();
};
