// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_InventorySlotWidget.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"

void UPZ_InventorySlotWidget::SetItemData(const UPZ_ItemDataAsset* ItemAsset)
{
	ClearSlot();
	
	if (!ItemAsset or ItemAsset->Icon.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid item asset for inventory slot widget"));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("SetItemData for item %s"), *ItemAsset->DisplayName.ToString()));
	IconLoadHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		ItemAsset->Icon.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this]()
		{
			if (UTexture2D* Texture = Cast<UTexture2D>(IconLoadHandle->GetLoadedAsset()))
			{
				ItemIconImage->SetBrushFromTexture(Texture);
				ItemIconImage->SetVisibility(ESlateVisibility::Visible);
			}
		})
	);
}

void UPZ_InventorySlotWidget::ClearSlot()
{
	if (IconLoadHandle.IsValid())
	{
		IconLoadHandle->CancelHandle();
		IconLoadHandle.Reset();
	}
	ItemIconImage->SetBrushFromTexture(nullptr);
	ItemIconImage->SetVisibility(ESlateVisibility::Hidden);
}
