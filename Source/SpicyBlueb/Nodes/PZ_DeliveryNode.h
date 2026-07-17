// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpicyBlueb/Core/Subsystems/PZ_ItemAssetRequester.h"
#include "SpicyBlueb/Interactables/PZ_Interactable.h"
#include "PZ_DeliveryNode.generated.h"

class UPZ_DeliveryNodeWidget;
class UWidgetComponent;
class UPZ_ItemDataAsset;
class APZ_ItemDummy;

USTRUCT(BlueprintType)
struct FPZ_DeliveryNodeOrder
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	int32 RequiredQuantity = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPrimaryAssetId RequiredItemId;
};

// Internal struct to track progress of orders
struct FPZ_DeliveryNodeOrderStatus
{
	FPZ_DeliveryNodeOrderStatus(int32 InRequiredQuantity, FPrimaryAssetId InRequiredItemId)
		: RequiredQuantity(InRequiredQuantity), ItemId(InRequiredItemId){}
	const int32 RequiredQuantity = 0;
	const FPrimaryAssetId ItemId;
	
	// Returns true if same asset id and count could be incremented. Returns false if count is finished
	bool TryAddToCount(FPrimaryAssetId AssetId)
	{
		if (ItemId != AssetId or ItemProgressCount >= RequiredQuantity)
			return false;
		
		ItemProgressCount++;
		return true;
	}	
	int32 GetProgressCount() const { return ItemProgressCount;}
private:
	int32 ItemProgressCount = 0;
};

UCLASS()
class SPICYBLUEB_API APZ_DeliveryNode : public AActor, public IPZ_Interactable
{
	GENERATED_BODY()

public:
	APZ_DeliveryNode();
	
	UFUNCTION(BlueprintCallable, Category="Delivery Node")
	void SetRequiredOrders(const TArray<FPZ_DeliveryNodeOrder>& InRequiredOrders);

	UFUNCTION()
	void OnPickupVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                const FHitResult& SweepResult);
	
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) override;
	virtual void OnInteractZoneEntered(APZ_PlayerCharacter* Interactor) override;
	virtual void OnInteractZoneExited(APZ_PlayerCharacter* Interactor) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Settings	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AcceptOrderDelay = 1.5f;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FPZ_DeliveryNodeOrder> DefaultRequiredOrders;
	TArray<FPZ_DeliveryNodeOrderStatus> RequiredOrders;
	
	// UI
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> StatusWidget;;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UPZ_DeliveryNodeWidget> StatusWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UPZ_DeliveryNodeWidget> SpawnedWidget;
	
	// Volume
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> PickupVolume;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UStaticMeshComponent> PickupVolumeMesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UPZ_InteractableVolumeComponent> InteractionVolume;
	
	
private:
	UFUNCTION()
	void OnPickupItemConfirmed(APZ_ItemDummy* Apz_ItemDummy);
	TMap<TWeakObjectPtr<APZ_ItemDummy>, FTimerHandle> AcceptOrderTimers;
	
	void RefreshUI();
	void LoadAllRequiredAssets();
	void CleanUp();
	
	void OnAssetsLoaded(TArray<FPrimaryAssetId> LoadedAssetIds);
	FPZ_ItemAssetRequester ItemRequester;
	TMap<FPrimaryAssetId, TWeakObjectPtr<UPZ_ItemDataAsset>> LoadedItemAssets;
};
