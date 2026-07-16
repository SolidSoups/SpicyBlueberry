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
	UFUNCTION()
	void OnPickupVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) override;
	virtual void OnInteractZoneEntered(APZ_PlayerCharacter* Interactor) override;
	virtual void OnInteractZoneExited(APZ_PlayerCharacter* Interactor) override;

	UPROPERTY()
	TArray<TWeakObjectPtr<APZ_ItemDummy>> PickupZoneItems;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Settings	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AcceptOrderDelay = 1.5f;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FPZ_DeliveryNodeOrder> RequiredOrders;
	
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
