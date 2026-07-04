// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_Shovel.h"

#include "Components/StaticMeshComponent.h"
#include "SpicyBlueb/Player/PZ_PlayerCharacter.h"


APZ_Shovel::APZ_Shovel()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
	Handle->SetupAttachment(RootComponent);
	
	Blade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Blade"));
	Blade->SetupAttachment(RootComponent);
	Blade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Blade->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Do we need a custom profile?
	Blade->OnComponentBeginOverlap.AddDynamic(this, &APZ_Shovel::OnBladeBeginOverlap);
}

void APZ_Shovel::SetBladeCollisionEnabled(bool IsEnabled)
{
	if (IsEnabled) HitActorsThisSwing.Empty();
	Blade->SetCollisionEnabled(IsEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void APZ_Shovel::OnBladeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APZ_PlayerCharacter* Wielder = Cast<APZ_PlayerCharacter>(GetOwner());
	ACharacter* Target = Cast<ACharacter>(OtherActor);
	
	if (!Wielder or !Target or Target == Wielder) return;
	if (HitActorsThisSwing.Contains(Target)) return;
	HitActorsThisSwing.Add(Target);
	
	const FVector LaunchDirection = Wielder->GetActorForwardVector().RotateAngleAxis(-LaunchAngle, Wielder->GetActorRightVector());
	Target->LaunchCharacter(LaunchDirection * LaunchForce, true, true);
}


