// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PZ_ItemData.h"
#include "Blueprint/UserWidget.h"
#include "PZ_InventorySlotWidget.generated.h"

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
	void SetItemData(const UPZ_ItemDataAsset* ItemAsset);
	void SetSelected(bool IsSelected);
	void ClearSlot();
	
protected:
	UPROPERTY(meta = (BindWidget))	
	UImage* ItemIconImage;
	
	// Unselected = Widget0, Selected = Widget1
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SlotImageSwitcher;
	
private:
	TSharedPtr<FStreamableHandle> IconLoadHandle;
};
