// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_GameMode_Menu.h"
#include "SpicyBlueb/Widgets/PZ_ConnectMenuWidget.h"

void APZ_GameMode_Menu::BeginPlay()
{
	Super::BeginPlay();
	
	if (UPZ_ConnectMenuWidget* MenuWidget = CreateWidget<UPZ_ConnectMenuWidget>(GetWorld(), MenuWidgetClass))
	{
		MenuWidget->AddToViewport();	
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
}
