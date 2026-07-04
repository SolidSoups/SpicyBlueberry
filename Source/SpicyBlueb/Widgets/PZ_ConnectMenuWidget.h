// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZ_ConnectMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class SPICYBLUEB_API UPZ_ConnectMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> SteamIDTextBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> HostButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ClientButton;
	
	UFUNCTION()
	void OnHostClicked();
	
	UFUNCTION()
	void OnClientClicked();
};
