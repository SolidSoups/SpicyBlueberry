// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventoryWidget.h"
#include "PZ_InventoryComponent.h"
#include "PZ_InventorySettings.h"
#include "PZ_InventorySlotWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "GameFramework/Pawn.h"

void UPZ_InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		InventoryComponent = OwningPawn->FindComponentByClass<UPZ_InventoryComponent>();	
	}
	
	if (!InventoryComponent or !SlotWidgetClass)
	{
		return;
	}
	
	InventoryComponent->OnItemLoadedDelegate.AddUObject(this, &UPZ_InventoryWidget::HandleItemLoaded);
	InventoryComponent->OnItemRemovedDelegate.AddUObject(this, &UPZ_InventoryWidget::HandleItemUnloaded);	

	int32 MaxItems = 2;
	if (const auto* Settings = GetDefault<UPZ_InventorySettings>())
	{
		MaxItems = Settings->MaxInventorySlots;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No max items for inventory slots in Settings"));
	}
	
	SlotVerticalBox->ClearChildren();
	SlotWidgets.Reset();
	SlotWidgets.Reserve(MaxItems);
	
	for (int32 i=0; i<MaxItems; i++)
	{
		UPZ_InventorySlotWidget* SlotWidget = CreateWidget<UPZ_InventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
		SlotVerticalBox->AddChildToVerticalBox(SlotWidget);
		SlotWidgets.Add(SlotWidget);
	}
}

void UPZ_InventoryWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPZ_InventoryWidget::HandleItemLoaded(const int32 SlotIndex, const UPZ_ItemDataAsset* ItemAsset)
{
	if (!SlotWidgets.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("Index invalid for SlotWidgets"));
		return;
	}
	
	SlotWidgets[SlotIndex]->SetItemData(ItemAsset); 
}

void UPZ_InventoryWidget::HandleItemUnloaded(int32 SlotIndex)
{
	if (!SlotWidgets.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("Index invalid for SlotWidgets"));
		return;
	}
	
	SlotWidgets[SlotIndex]->ClearSlot();
}
