// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PZ_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


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
}

void APZ_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	AddInputMapping();
}

void APZ_PlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bUsingGamepadAim)
	{
		UpdateMouseFacing();
	}

	ApplyFacing(DeltaTime);
}

void APZ_PlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	AddInputMapping();
}

void APZ_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	
	if (!EIC) return;
	if (MoveAction) EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APZ_PlayerCharacter::Move);
	if (AimAction) EIC->BindAction(AimAction, ETriggerEvent::Triggered, this, &APZ_PlayerCharacter::Aim);
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
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
	const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	DesiredFacing = (Forward * Stick.Y + Right * Stick.X).GetSafeNormal();
}

void APZ_PlayerCharacter::UpdateMouseFacing()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController()) return;
	
	FVector WorldOrigin, WorldDir;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDir)) return;

	const FVector CharLoc = GetActorLocation();
	
	const FPlane GroundPlane(CharLoc, FVector::UpVector);
	const FVector RayEnd = WorldOrigin + WorldDir * 10000000.f;

	FVector HitPoint;
	if (!FMath::SegmentPlaneIntersection(WorldOrigin, RayEnd, GroundPlane, HitPoint)) return;

	FVector ToCursor = HitPoint - CharLoc;
	ToCursor.Z = 0.f;

	if (!ToCursor.IsNearlyZero())
	{
		DesiredFacing = ToCursor.GetSafeNormal();
	}
	
	if(bDebug)
	{
		DrawDebugLine(GetWorld(), CharLoc, CharLoc + DesiredFacing * 300.f, FColor::Red, false, -1.f, 0, 3.f);
	}
}

void APZ_PlayerCharacter::ApplyFacing(float DeltaTime)
{
	if (DesiredFacing.IsNearlyZero()) return;

	const float TargetYaw = DesiredFacing.Rotation().Yaw;
	SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
}

void APZ_PlayerCharacter::AddInputMapping()
{
	if (APZ_PlayerController* PC = Cast<APZ_PlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* EILPS = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					EILPS->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}
