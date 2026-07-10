// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZ_InventoryWidget.generated.h"

class UVerticalBox;
class UPZ_InventoryComponent;
class UPZ_ItemDataAsset;
class UPZ_InventorySlotWidget;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class SPICYBLUEB_API UPZ_InventoryWidget : public UUserWidget
{

private:
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void HandleItemLoaded(const int32 SlotIndex, const UPZ_ItemDataAsset* ItemAsset);
	UFUNCTION()
	void HandleItemUnloaded(int32 SlotIndex);
	
	UPROPERTY()
	TObjectPtr<UPZ_InventoryComponent> InventoryComponent;
	
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* SlotVerticalBox;
	
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UPZ_InventorySlotWidget> SlotWidgetClass;
	
	UPROPERTY()
	TArray<UPZ_InventorySlotWidget*> SlotWidgets;
};
