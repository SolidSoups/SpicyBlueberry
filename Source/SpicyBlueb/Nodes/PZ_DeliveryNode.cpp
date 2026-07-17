// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_DeliveryNode.h"

#include "PZ_DeliveryNodeWidget.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "SpicyBlueb/Interactables/PZ_InteractableVolumeComponent.h"
#include "SpicyBlueb/Inventory/PZ_ItemData.h"
#include "SpicyBlueb/Inventory/PZ_ItemDummy.h"


APZ_DeliveryNode::APZ_DeliveryNode()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupVolumeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Volume Mesh");
	SetRootComponent(PickupVolumeMesh);

	PickupVolume = CreateDefaultSubobject<USphereComponent>(TEXT("Pickup Volume"));
	PickupVolume->SetupAttachment(PickupVolumeMesh);
	PickupVolume->SetSphereRadius(50.f);
	PickupVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	PickupVolume->OnComponentBeginOverlap.AddDynamic(this, &APZ_DeliveryNode::OnPickupVolumeBeginOverlap);

	StatusWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget Component"));
	StatusWidget->SetupAttachment(RootComponent);
	StatusWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	StatusWidget->SetWidgetSpace(EWidgetSpace::World);
	StatusWidget->SetDrawSize(FVector2D(200.f, 200.f));

	InteractionVolume = CreateDefaultSubobject<UPZ_InteractableVolumeComponent>(TEXT("Interaction Volume Component"));
	InteractionVolume->RegisterVolume(PickupVolume);
}

void APZ_DeliveryNode::SetRequiredOrders(const TArray<FPZ_DeliveryNodeOrder>& InRequiredOrders)
{
	DefaultRequiredOrders = InRequiredOrders;
	CleanUp();
	LoadAllRequiredAssets();
}

void APZ_DeliveryNode::OnPickupVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                  const FHitResult& SweepResult)
{
	if (auto* Dummy = Cast<APZ_ItemDummy>(OtherActor))
	{
		GetWorldTimerManager().SetTimer(
			AcceptOrderTimers.FindOrAdd(Dummy),
			FTimerDelegate::CreateUObject(this, &APZ_DeliveryNode::OnPickupItemConfirmed, Dummy),
			AcceptOrderDelay,
			false
		);
	}
}

void APZ_DeliveryNode::BeginPlay()
{
	Super::BeginPlay();

	if (StatusWidgetClass)
	{
		StatusWidget->SetWidgetClass(StatusWidgetClass);
		SpawnedWidget = Cast<UPZ_DeliveryNodeWidget>(StatusWidget->GetWidget());
	}

	if (!DefaultRequiredOrders.IsEmpty())
	{
		// Transfer into array	
		RequiredOrders.Reset();
		for (const auto& [RequiredQuantity, RequiredAssetId] : DefaultRequiredOrders)
		{
			if (!ensureMsgf(RequiredAssetId.IsValid(), TEXT("Required asset is not valid!"))
				or !ensureMsgf(RequiredQuantity > 0, TEXT("Required asset must have a count larger than 0")))
				continue;
			RequiredOrders.Add({RequiredQuantity, RequiredAssetId});
		}

		LoadAllRequiredAssets();
	}
}


void APZ_DeliveryNode::LoadAllRequiredAssets()
{
	// batch all unique asset ids together and load them together
	TArray<FPrimaryAssetId> AssetIdsToLoad;
	for (const auto& Order : RequiredOrders)
	{
		AssetIdsToLoad.AddUnique(Order.ItemId);	
	}

	ItemRequester.LoadBatch(GetWorld(), AssetIdsToLoad,
	                        FOnAssetBatchLoadedDelegate::CreateUObject(this, &APZ_DeliveryNode::OnAssetsLoaded));
}

void APZ_DeliveryNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	CleanUp();
}

void APZ_DeliveryNode::CleanUp()
{
	// Release assets
	ItemRequester.ReleaseAll(GetWorld());
	LoadedItemAssets.Reset();

	// Clean up all timers
	for (auto& [Dummy,TimerHandle] : AcceptOrderTimers)
	{
		if (TimerHandle.IsValid())
		{
			GetWorldTimerManager().ClearTimer(TimerHandle);
		}
	}
	AcceptOrderTimers.Reset();

	// Clear widget
	if (SpawnedWidget)
		SpawnedWidget->ClearState();
}

void APZ_DeliveryNode::OnInteract(APZ_PlayerCharacter* Interactor)
{
	// does the player fulfill the required orders?
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("DeliveryNode::OnInteract!"));
}

void APZ_DeliveryNode::OnInteractZoneEntered(APZ_PlayerCharacter* Interactor)
{
	// display orders, cross off the ones the player has
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("DeliveryNode::OnZoneEntered"));
}

void APZ_DeliveryNode::OnInteractZoneExited(APZ_PlayerCharacter* Interactor)
{
	// hide order UI
	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("DeliveryNode::OnZoneExited"));
}


void APZ_DeliveryNode::OnAssetsLoaded(TArray<FPrimaryAssetId> LoadedAssetIds)
{
	LoadedItemAssets.Reset();
	for (const FPrimaryAssetId& AssetId : LoadedAssetIds)
	{
		LoadedItemAssets.Add(AssetId, Cast<UPZ_ItemDataAsset>(
			                     UAssetManager::Get().GetPrimaryAssetObject(AssetId)));
	}
	RefreshUI();
}

void APZ_DeliveryNode::RefreshUI()
{
	// Build the data for the UI to consume	
	TArray<FPZ_WidgetOrderInfo> WidgetOrderInfos;
	for (const auto& Order : RequiredOrders)
	{
		if (!Order.ItemId.IsValid()) // something weird happened
			continue;

		FSoftObjectPath IconPath;
		if (auto ItemAsset = LoadedItemAssets.FindRef(Order.ItemId); ItemAsset.IsValid())
			IconPath = ItemAsset->Icon.ToSoftObjectPath();
		
		// We add ProgressCount fulfilled orders, rest are unfulfilled
		int32 ProgressCount = Order.GetProgressCount();
		for (int32 i = 0; i < Order.RequiredQuantity; i++)
		{
			WidgetOrderInfos.Add({i < ProgressCount, IconPath});
		}
	}

	SpawnedWidget->UpdateOrderImages(WidgetOrderInfos);
}

void APZ_DeliveryNode::OnPickupItemConfirmed(APZ_ItemDummy* ItemDummy)
{
	if (ItemDummy)
	{
		// Check if this item can progress any of our required orders
		for (auto& RequiredOrder : RequiredOrders)
		{
			if (RequiredOrder.TryAddToCount(ItemDummy->ItemId))
			{
				ItemDummy->Destroy();
				break;
			}
		}

		// Clean up timer handle
		if (FTimerHandle* Handle = AcceptOrderTimers.Find(ItemDummy))
		{
			GetWorldTimerManager().ClearTimer(*Handle);
			AcceptOrderTimers.Remove(ItemDummy);
		}
	}
	RefreshUI();
}
