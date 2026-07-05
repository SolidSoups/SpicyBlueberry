// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_ConnectMenuWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UPZ_ConnectMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HostButton->OnClicked.AddDynamic(this, &UPZ_ConnectMenuWidget::OnHostClicked);
	ClientButton->OnClicked.AddDynamic(this, &UPZ_ConnectMenuWidget::OnClientClicked);
}

void UPZ_ConnectMenuWidget::OnHostClicked()
{
	// Start a host server	
	GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("open /Game/Maps/PZ_LVL_PlayerTesting?listen"));
}

void UPZ_ConnectMenuWidget::OnClientClicked()
{
	// Parse steam id
	FString SteamID = SteamIDTextBox->GetText().ToString();
	FString Command = FString::Printf(TEXT("open steam.%s"), *SteamID);
	
	// Connect as a client to the steam id host
	GetWorld()->GetFirstPlayerController()->ConsoleCommand(Command);
}
