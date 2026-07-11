// Fill out your copyright notice in the Description page of Project Settings.

#include "PZ_PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PZ_PlayerController.h"
#include "Animation/AnimInstance.h"
#include "PZ_PlayerState.h"
#include "BaseGizmos/GizmoElementShared.h"
#include "Camera/CameraComponent.h"
#include "Chaos/AABBTree.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "SpicyBlueb/Core/GameMode/PZ_GameModeBase.h"
#include "SpicyBlueb/Delivery/PZ_DeliveryWorldSubsystem.h"
#include "SpicyBlueb/Delivery/PZ_Restaurant.h"
#include "SpicyBlueb/Inventory/PZ_InventoryComponent.h"
#include "SpicyBlueb/Inventory/PZ_InventorySettings.h"
#include "SpicyBlueb/Inventory/PZ_ItemData.h"
#include "SpicyBlueb/Inventory/PZ_ItemDummy.h"
#include "SpicyBlueb/Pizza/PZ_Pizza.h"
#include "SpicyBlueb/Weapons/PZ_Shovel.h"


APZ_PlayerCharacter::APZ_PlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = CameraHeight;
	SpringArm->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	InventoryComponent = CreateDefaultSubobject<UPZ_InventoryComponent>(TEXT("Inventory Component"));
	
	ItemDropPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Item Drop Point"));
	ItemDropPoint->SetupAttachment(GetMesh());
}

void APZ_PlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// OnConstruct runs during editor runtime, so we need to change socket
	// settings here
	SpringArm->TargetArmLength = CameraHeight;
	SpringArm->SetRelativeRotation(FRotator(CameraPitch, CameraYaw, 0.f));
}

void APZ_PlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APZ_PlayerCharacter, RepFacingYaw, COND_SkipOwner)
	DOREPLIFETIME_CONDITION(APZ_PlayerCharacter, RepAttackTrigger, COND_SkipOwner);
	DOREPLIFETIME(APZ_PlayerCharacter, CarriedPizza);
}

void APZ_PlayerCharacter::PlayAttackMontage()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void APZ_PlayerCharacter::SetWeaponCollisionEnabled(bool IsEnabled)
{
	if (EquippedShovel) EquippedShovel->SetHitVolumeEnabled(IsEnabled);
}

FVector APZ_PlayerCharacter::GetFacingDirection() const
{
	if (IsLocallyControlled())
		return FRotator(0.f, LastAppliedYaw, 0.f).Vector();

	return FRotator(0.f, RepFacingYaw, 0.f).Vector();
}

void APZ_PlayerCharacter::SetOverlappingItemPickup(APZ_ItemDummy* Pickup)
{
	OverlappingItemPickup = Pickup;
	OnInteractableChanged.Broadcast(Pickup ? Pickup->ItemId : FPrimaryAssetId{});
}

void APZ_PlayerCharacter::ClearOverlappingItemPickup(APZ_ItemDummy* Pickup)
{
	if (OverlappingItemPickup != Pickup) return;
	
	OverlappingItemPickup = nullptr;
	OnInteractableChanged.Broadcast(FPrimaryAssetId{});
}


void APZ_PlayerCharacter::CarryPizza(APZ_Pizza* Pizza)
{
	if (!HasAuthority() || !Pizza) return;

	CarriedPizza = Pizza;
	Pizza->PickUp(GetMesh(), PizzaSocketName);
}

void APZ_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	AddInputMappings();
	SpawnAndAttachShovel();
	LastAppliedYaw = GetActorRotation().Yaw;
}

void APZ_PlayerCharacter::DrawHeadingToNearestDeliveryPoint()
{
	UPZ_DeliveryWorldSubsystem* DeliverySS = GetWorld()->GetSubsystem<UPZ_DeliveryWorldSubsystem>();
	if (!DeliverySS)
		return;

	// Find the closest delivery location. Probably the most obnoxious way to do it
	TArray<FVector> DeliveryLocations = DeliverySS->GetDeliveryLocations();
	if (DeliveryLocations.Num() == 0) return;

	const FVector MyLocation = GetActorLocation();
	const FVector* Closest = Algo::MinElementBy(DeliveryLocations,
	                                            [MyLocation](const FVector& Loc)
	                                            {
		                                            return FVector::DistSquared(MyLocation, Loc);
	                                            });

	// Did we find a closest location?
	if (Closest)
	{
		const FVector Heading = (*Closest - MyLocation).GetSafeNormal2D();
		DrawDebugDirectionalArrow(GetWorld(), MyLocation, MyLocation + Heading * 200.f, 50.f, FColor::Yellow, false,
		                          0.f, 0U, 5.f);
	}
}

void APZ_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bUsingGamepadAim)
	{
		UpdateMouseFacing();
	}

	ApplyFacing(DeltaTime);


	// # Debug tell us nearest delivery point

	DrawHeadingToNearestDeliveryPoint();
}

void APZ_PlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddInputMappings();
}

void APZ_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput) return;
	
	if (MoveAction) EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APZ_PlayerCharacter::Move);
	if (AimAction) EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &APZ_PlayerCharacter::Aim);
	if (JumpAction)
	{
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (AttackAction)
		EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &APZ_PlayerCharacter::DoAttack);
	if (InteractAction)
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &APZ_PlayerCharacter::Interact);
	
	if (DropItemAction)
		EnhancedInput->BindAction(DropItemAction, ETriggerEvent::Started, this, &APZ_PlayerCharacter::DropItem);
	
	// Bind inventory slot actions
	const auto* InventorySettings = GetDefault<UPZ_InventorySettings>();
	const int32 MinNumSlotActions = FMath::Min(InventorySettings->MaxInventorySlots, SelectedSlotActions.Num());
	for (int32 i = 0; i < MinNumSlotActions; ++i)
	{
		if (SelectedSlotActions[i])
		{
			EnhancedInput->BindAction(
				SelectedSlotActions[i],
				ETriggerEvent::Started,
				this,
				&APZ_PlayerCharacter::SelectInventorySlot, i);
		}	
	}
}


void APZ_PlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	const FRotator YawRot(0.f, CameraYaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void APZ_PlayerCharacter::Aim(const FInputActionValue& Value)
{
	const FVector2D Stick = Value.Get<FVector2D>();

	if (Stick.SizeSquared() < GamepadAimDeadzone * GamepadAimDeadzone)
	{
		bUsingGamepadAim = false;
		return;
	}

	bUsingGamepadAim = true;

	const FRotator YawRot(0.f, CameraYaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	DesiredFacing = (Forward * Stick.Y + Right * Stick.X).GetSafeNormal();
}

void APZ_PlayerCharacter::DoAttack()
{
	UE_LOG(LogTemp, Display, TEXT("Player Attack!"));
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage)) return;
	}

	PlayAttackMontage();

	if (HasAuthority())
	{
		RepAttackTrigger++;
		MARK_PROPERTY_DIRTY_FROM_NAME(APZ_PlayerCharacter, RepAttackTrigger, this);
	}
	else
	{
		Server_DoAttack(LastAppliedYaw);
	}
}

void APZ_PlayerCharacter::Interact()
{
	if (HasAuthority())
	{
		Server_Interact_Implementation();
	}
	else
	{
		Server_Interact();
	}
}

void APZ_PlayerCharacter::DropItem()
{
	if (!InventoryComponent)
		return;
	
	int32 SelectedSlot = InventoryComponent->GetSelectedSlot();
	UPZ_ItemDataAsset* ItemData = InventoryComponent->GetItemData(SelectedSlot);
	if (!ItemData) return; // no item here
	
	// spawn dummy actor 
	if (ItemData->DummyActorClass)
	{
		FVector SpawnLocation = ItemDropPoint->GetComponentLocation();		
		FRotator SpawnRotation = ItemDropPoint->GetComponentRotation();
		UClass* DummyClass = ItemData->DummyActorClass.LoadSynchronous(); // TODO: Better way to load the class
		APZ_ItemDummy* NewActor = GetWorld()->SpawnActor<APZ_ItemDummy>(DummyClass, SpawnLocation, SpawnRotation);
		
		// add some drop force
		if (NewActor)
			if (auto* PhysicsComp = Cast<UPrimitiveComponent>(NewActor->GetRootComponent()))
			{
				PhysicsComp->AddImpulse(GetFacingDirection() * DropImpulseStrength + GetVelocity(), NAME_None, true);
			}
	}
	
	InventoryComponent->TryPopItem(SelectedSlot);
}

void APZ_PlayerCharacter::SelectInventorySlot(int32 Slot)
{
	if (InventoryComponent)
		InventoryComponent->SelectSlot(Slot);
}

void APZ_PlayerCharacter::SpawnAndAttachShovel()
{
	if (!ShovelClass) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	EquippedShovel = GetWorld()->SpawnActor<APZ_Shovel>(ShovelClass, GetActorTransform(), Params);

	if (EquippedShovel)
	{
		EquippedShovel->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			HandSocketName);
	}
}

void APZ_PlayerCharacter::UpdateMouseFacing()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController()) return;

	FVector WorldOrigin, WorldDir;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDir)) return;

	const FVector CharLocation = GetActorLocation();

	const FPlane GroundPlane(CharLocation, FVector::UpVector);
	const FVector RayEnd = WorldOrigin + WorldDir * 10000000.f;

	FVector HitPoint;
	if (!FMath::SegmentPlaneIntersection(WorldOrigin, RayEnd, GroundPlane, HitPoint)) return;

	FVector ToCursor = HitPoint - CharLocation;
	ToCursor.Z = 0.f;

	if (!ToCursor.IsNearlyZero())
	{
		DesiredFacing = ToCursor.GetSafeNormal();
	}

	if (bDebug)
	{
		DrawDebugLine(GetWorld(), CharLocation, CharLocation + DesiredFacing * 300.f, FColor::Red, false, -1.f, 0, 3.f);
	}
}

void APZ_PlayerCharacter::ApplyFacing(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		if (DesiredFacing.IsNearlyZero()) return;
		const float TargetYaw = DesiredFacing.Rotation().Yaw;
		SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
		LastAppliedYaw = TargetYaw;

		if (HasAuthority())
		{
			RepFacingYaw = TargetYaw;
			MARK_PROPERTY_DIRTY_FROM_NAME(APZ_PlayerCharacter, RepFacingYaw, this);
		}
		else
		{
			Server_SetFacingYaw(TargetYaw);
		}
		return;
	}

	if (HasAuthority())
	{
		// Smooth the new yaw
		const float NewYaw = FMath::FixedTurn(LastAppliedYaw, RepFacingYaw, AimRotationSpeed * DeltaTime);
		SetActorRotation(FRotator(0.f, NewYaw, 0.f));
		LastAppliedYaw = NewYaw;
	}

	// Purely simulated proxies (not owner, not server) do nothing here.
	// ACharacter already replicated and smooths rotation
}

void APZ_PlayerCharacter::OnRep_AttackTrigger()
{
	PlayAttackMontage();
}

void APZ_PlayerCharacter::Server_Interact_Implementation()
{
	// Always pick up items first
	if (OverlappingItemPickup)
	{
		APZ_ItemDummy* Pickup = OverlappingItemPickup;
		if (InventoryComponent->AddItem(Pickup->ItemId))
		{
			ClearOverlappingItemPickup(Pickup);	
			Pickup->Destroy();
			return;
		}
	}	
	
	// Carrying -> try to deliver at the delivery point I'm currently on.
	if (CarriedPizza)
	{
		if (OverlappingDeliveryPoint)
		{
			APZ_PlayerState* PS = GetPlayerState<APZ_PlayerState>();
			if (PS)
			{
				if (UPZ_DeliveryWorldSubsystem* DeliverySS = GetWorld()->GetSubsystem<UPZ_DeliveryWorldSubsystem>())
				{
					if (DeliverySS->TryDeliver(PS, OverlappingDeliveryPoint, CarriedPizza) > 0)
						ClearCarriedPizza();
				}
			}
		}
		else if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Not on a delivery point"));
		}
		return;
	}

	// Empty-handed -> grab a pizza from the restaurant I'm standing in.
	if (OverlappingRestaurant)
	{
		OverlappingRestaurant->RequestPizza(this);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Not in a restaurant"));
	}
}

void APZ_PlayerCharacter::Server_DoAttack_Implementation(float ClientFacingYaw)
{
	// we want all clients to use this Yaw for the attack calculations
	RepFacingYaw = ClientFacingYaw;
	MARK_PROPERTY_DIRTY_FROM_NAME(APZ_PlayerCharacter, RepFacingYaw, this);

	PlayAttackMontage();

	RepAttackTrigger++;
	MARK_PROPERTY_DIRTY_FROM_NAME(APZ_PlayerCharacter, RepAttackTrigger, this);
}

void APZ_PlayerCharacter::Server_SetFacingYaw_Implementation(float NewYaw)
{
	RepFacingYaw = NewYaw;
	// Mark RepFacingYaw as dirty. Iris keeps track of all dirty properties
	// in an object-to-property table. We need to explicitly trigger the replication
	// by setting it to dirty.
	MARK_PROPERTY_DIRTY_FROM_NAME(APZ_PlayerCharacter, RepFacingYaw, this);
}

void APZ_PlayerCharacter::AddInputMappings()
{
	APZ_PlayerController* PlayerController = Cast<APZ_PlayerController>(GetController());
	if (!PlayerController) return;
	
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	auto* EnhancedInputSS = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!EnhancedInputSS) return;

	if (DefaultMappingContext)
		EnhancedInputSS->AddMappingContext(DefaultMappingContext, 0);
	if (InventoryMappingContext)
		EnhancedInputSS->AddMappingContext(InventoryMappingContext, 1);
}