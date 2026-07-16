// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PZ_DeliveryNodeWidget.generated.h"

struct FStreamableHandle;
class UPZ_ItemDataAsset;
class UWrapBox;

USTRUCT()
struct FPZ_WidgetOrderInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool IsFulfilled = false;
	
	UPROPERTY()
	FSoftObjectPath IconPath;	
};

/**
 * 
 */
UCLASS()
class SPICYBLUEB_API UPZ_DeliveryNodeWidget : public UUserWidget
{

private:
	GENERATED_BODY()

public:
	void UpdateOrderImages(TArray<FPZ_WidgetOrderInfo>& Orders);
	
protected:
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta = (BindWidget))
	UWrapBox* OrderWrapBox;	
	
	UPROPERTY(EditAnywhere, Category = "Order Icons")
	float OrderIconSize = 280.f;

	UPROPERTY(EditAnywhere, Category = "Order Icons")
	FMargin OrderIconPadding = FMargin(4.f);
	
private:
	TArray<TSharedPtr<FStreamableHandle>> IconLoadHandles;
};
