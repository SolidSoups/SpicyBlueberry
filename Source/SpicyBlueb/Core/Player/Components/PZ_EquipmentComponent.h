// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PZ_EquipmentComponent.generated.h"


class UPZ_ItemDataAsset;
class APZ_EquippableActor;
class UPZ_InventoryComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SPICYBLUEB_API UPZ_EquipmentComponent : public UActorComponent
{

private:
	GENERATED_BODY()

public:
	UPZ_EquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	APZ_EquippableActor* GetCurrentEquipped() const { return CurrentEquipped; }
	
protected:
	void RefreshEquippedItemFromSlot();
	void EquipItem(UPZ_ItemDataAsset* ItemData);
	USkeletalMeshComponent* GetOwnerSkeletalMesh() const;
	
	UPROPERTY()
	TObjectPtr<UPZ_InventoryComponent> InventoryComponent;
	
	UPROPERTY(Replicated)
	TObjectPtr<APZ_EquippableActor> CurrentEquipped;
	
	UPROPERTY()
	TObjectPtr<UPZ_ItemDataAsset> CurrentEquippedItemData;
};
