// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_ItemResourceManagerComponent.h"

#include "PersonaAssetEditorToolkit.h"
#include "Engine/AssetManager.h"
#include "SpicyBlueb/Inventory/PZ_ItemData.h"

UPZ_ItemResourceManagerComponent::UPZ_ItemResourceManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPZ_ItemResourceManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPZ_ItemResourceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}