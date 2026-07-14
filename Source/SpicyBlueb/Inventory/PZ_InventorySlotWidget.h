// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZ_ItemData.h"
#include "Blueprint/UserWidget.h"
#include "PZ_InventorySlotWidget.generated.h"

class UTextBlock;
class UWidgetSwitcher;
class UImage;
struct FStreamableHandle;
/**
 * 
 */
UCLASS()
class SPICYBLUEB_API UPZ_InventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Initialize(int32 SlotIndex);
	/* Loads the icon image into memory and enabled visibility of that icon */
	void SetItemData(const UPZ_ItemDataAsset* ItemAsset);
	/* Triggers a WidgetSwitcher to switch to either a dark (false) or light (true) background */
	void SetSelected(bool IsSelected);
	/* Unloads icon from memory and disables its visibility */
	void ClearSlot();
	
protected:
	UPROPERTY(meta = (BindWidget))	
	UImage* ItemIconImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SlotTextBlock;
	
	// Unselected = Widget0, Selected = Widget1
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SlotImageSwitcher;
	
private:
	TSharedPtr<FStreamableHandle> IconLoadHandle;
};
