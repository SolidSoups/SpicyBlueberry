// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_Shovel.h"

#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"

APZ_Shovel::APZ_Shovel()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	Handle->SetupAttachment(RootComponent);
	Handle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Blade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Blade"));
	Blade->SetupAttachment(RootComponent);
	Blade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HitVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("HitVolume"));
	HitVolume->SetupAttachment(Blade);
	HitVolume->SetBoxExtent(FVector(10.f, 10.f, 20.f));
	HitVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HitVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitVolume->OnComponentBeginOverlap.AddDynamic(this, &APZ_Shovel::OnHitVolumeBeginOverlap);
}

void APZ_Shovel::SetHitVolumeEnabled(bool IsEnabled)
{
	if (IsEnabled) HitActorsThisSwing.Empty();
	HitVolume->SetCollisionEnabled(IsEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

FVector APZ_Shovel::ComputeLaunchDirection(const FVector& Forward2D) const
{
	const float AngleRad = FMath::DegreesToRadians(LaunchAngle);
	return (Forward2D * FMath::Cos(AngleRad) + FVector::UpVector * FMath::Sin(AngleRad)).GetSafeNormal();
}

void APZ_Shovel::OnHitVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	APZ_PlayerCharacter* Wielder = Cast<APZ_PlayerCharacter>(GetOwner());
	ACharacter* Target = Cast<ACharacter>(OtherActor);

	if (!Wielder or !Target or Target == Wielder) return;
	
	// Only hit other actors once
	if (HitActorsThisSwing.Contains(Target)) return;
	HitActorsThisSwing.Add(Target);
	
	// Only apply physics on the server or a pawn that you control!	
	if (Wielder->HasAuthority() or Target->IsLocallyControlled())
	{
		FVector LaunchVelocity = ComputeLaunchDirection(Wielder->GetFacingDirection().GetSafeNormal2D()) * LaunchForce;
		Target->LaunchCharacter(LaunchVelocity, true, true);
	}
	else
	{
		// TODO: add attacker feedback, hit sound, particle, camera shake
	}
	
	if (Wielder->HasAuthority())
	{
		// Mutate authoritative state that clients should NOT touch
	}
}
