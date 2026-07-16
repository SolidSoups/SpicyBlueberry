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
	PickupVolume->OnComponentEndOverlap.AddDynamic(this, &APZ_DeliveryNode::OnPickupVolumeEndOverlap);

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
	RequiredOrders = InRequiredOrders;
	CleanUp();
	LoadAllRequiredAssets();
}

void APZ_DeliveryNode::OnPickupVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                  const FHitResult& SweepResult)
{
	if (auto* Dummy = Cast<APZ_ItemDummy>(OtherActor))
	{
		PickupZoneItems.AddUnique(Dummy);
		GetWorldTimerManager().SetTimer(
			AcceptOrderTimers.FindOrAdd(Dummy),
			FTimerDelegate::CreateUObject(this, &APZ_DeliveryNode::OnPickupItemConfirmed, Dummy),
			AcceptOrderDelay,
			false
		);
	}
}

void APZ_DeliveryNode::OnPickupVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto* Dummy = Cast<APZ_ItemDummy>(OtherActor))
	{
		if (FTimerHandle* Handle = AcceptOrderTimers.Find(Dummy))
		{
			GetWorldTimerManager().ClearTimer(*Handle);
			AcceptOrderTimers.Remove(Dummy);
		}
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

	if (!RequiredOrders.IsEmpty())
		LoadAllRequiredAssets();
}


void APZ_DeliveryNode::LoadAllRequiredAssets()
{
	// batch all asset ids together and load them together
	for (int32 i = 0; i < RequiredOrders.Num(); i++)
	{
		LoadedAssetIds.AddUnique(RequiredOrders[i].RequiredItemId);
	}
	UAssetManager::Get().LoadPrimaryAssets(LoadedAssetIds,
	                                       TArray<FName>(),
	                                       FStreamableDelegate::CreateUObject(this, &APZ_DeliveryNode::OnAssetsLoaded));
}

void APZ_DeliveryNode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	CleanUp();
}

void APZ_DeliveryNode::CleanUp()
{
	// Clean up all loaded assets
	if (!LoadedAssetIds.IsEmpty())
	{
		UAssetManager::Get().UnloadPrimaryAssets(LoadedAssetIds);
		LoadedAssetIds.Reset();
	}
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


void APZ_DeliveryNode::RefreshUI()
{
	// Count up the totals for each asset id (there may be several of the same item in the zone)
	TMap<FPrimaryAssetId, int32> AssetIdCounts;
	for (const TWeakObjectPtr<APZ_ItemDummy>& ItemDummy : PickupZoneItems)
	{
		if (!ItemDummy.IsValid()) continue;
		AssetIdCounts.FindOrAdd(ItemDummy->ItemId)++;
	}


	// Build the data for the UI to consume	
	TArray<FPZ_WidgetOrderInfo> WidgetOrderInfos;
	for (const auto& [RequiredQuantity, RequiredItemId] : RequiredOrders)
	{
		if (!RequiredItemId.IsValid() or RequiredQuantity <= 0)
			continue;

		FSoftObjectPath IconPath;
		if (auto ItemAsset = LoadedItemAssets.FindRef(RequiredItemId); ItemAsset.IsValid())
			IconPath = ItemAsset->Icon.ToSoftObjectPath();

		int32 Count = AssetIdCounts.FindRef(RequiredItemId);
		for (int32 i = 0; i < RequiredQuantity; i++)
		{
			WidgetOrderInfos.Add({i < Count, IconPath});
		}

		if (int32* CountPtr = AssetIdCounts.Find(RequiredItemId))
			*CountPtr -= FMath::Max(0, static_cast<int32>(FMath::Min(Count, RequiredQuantity)));
	}

	SpawnedWidget->UpdateOrderImages(WidgetOrderInfos);
}

void APZ_DeliveryNode::OnPickupItemConfirmed(APZ_ItemDummy* ItemDummy)
{
	PickupZoneItems.Add(ItemDummy);	
	RefreshUI();
}

void APZ_DeliveryNode::OnAssetsLoaded()
{
	LoadedItemAssets.Reset();
	for (const FPrimaryAssetId& AssetId : LoadedAssetIds)
	{
		LoadedItemAssets.Add(AssetId, Cast<UPZ_ItemDataAsset>(UAssetManager::Get().GetPrimaryAssetObject(AssetId)));
	}
	RefreshUI();
}
