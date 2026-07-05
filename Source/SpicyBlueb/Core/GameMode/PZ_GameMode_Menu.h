// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PZ_GameMode_Menu.generated.h"

/**
 * 
 */
UCLASS()
class SPICYBLUEB_API APZ_GameMode_Menu : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPZ_ConnectMenuWidget> MenuWidgetClass;
};
