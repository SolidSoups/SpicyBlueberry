// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_DeliveryNodeWidget.h"

#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "SpicyBlueb/Inventory/PZ_ItemData.h"

void UPZ_DeliveryNodeWidget::UpdateOrderImages(TArray<FPZ_WidgetOrderInfo>& Orders)
{
	ClearState();
	
	// Construct a shit ton of images for every order required
	for (const auto& Order : Orders)
	{
		TWeakObjectPtr<UImage> WeakIconImage = NewObject<UImage>(this);
		if (UWrapBoxSlot* WrapBoxSlot = OrderWrapBox->AddChildToWrapBox(WeakIconImage.Get()))
		{
			WrapBoxSlot->SetPadding(OrderIconPadding);
		}
		
		const FSoftObjectPath IconPath = Order.IconPath;
		
		FVector2D IconSize(OrderIconSize);
		bool IsFulfilled = Order.IsFulfilled;
		IconLoadHandles.Add(UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			IconPath,
			FStreamableDelegate::CreateWeakLambda(this, [WeakIconImage, IconSize, IconPath, IsFulfilled]()
			{
				if (UImage* Image = WeakIconImage.Get())
				{
					if (UTexture2D* Texture = Cast<UTexture2D>(IconPath.ResolveObject()))
					{
						Image->SetBrushFromTexture(Texture);
						Image->SetBrushSize(IconSize);
						if (IsFulfilled)
						{
							Image->SetBrushTintColor(FSlateColor(FColor(255, 255, 255, 120)));	
						}
					}
				}
			})
		));
	}
}

void UPZ_DeliveryNodeWidget::ClearState()
{
	OrderWrapBox->ClearChildren();
	
	// Clear all icon resources
	for (const TSharedPtr<FStreamableHandle>& Handle : IconLoadHandles)
	{
		if (Handle.IsValid())
		{
			Handle->CancelHandle();
		}	
	}
	IconLoadHandles.Reset();
}

void UPZ_DeliveryNodeWidget::NativeDestruct()
{
	Super::NativeDestruct();
	ClearState();
}
