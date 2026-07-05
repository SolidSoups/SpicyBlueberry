#include "PZ_Pizza.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


APZ_Pizza::APZ_Pizza()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	SetReplicateMovement(true);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PizzaMesh"));
	SetRootComponent(Mesh);
	
	Mesh->SetSimulatePhysics(true);
	Mesh->SetLinearDamping(2.f);
	Mesh->SetMassOverrideInKg(NAME_None, 2.f,true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APZ_Pizza::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APZ_Pizza, Quality);
	DOREPLIFETIME(APZ_Pizza, bCarried);
}

void APZ_Pizza::PickUp(USceneComponent* AttachTo, FName SocketName)
{
	if (!HasAuthority() || !AttachTo) return;

	bCarried = true;
	ApplyCarriedPhysicsState();


	SetReplicateMovement(false);
	AttachToComponent(AttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

	OnRep_Carried();
}

void APZ_Pizza::ThrowOrDrop(const FVector& Impulse)
{
	if (!HasAuthority()) return;

	bCarried = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	ApplyCarriedPhysicsState();
	SetReplicateMovement(true);

	if (!Impulse.IsNearlyZero())
	{
		Mesh->AddImpulse(Impulse, NAME_None, true);
	}

	OnRep_Carried();
}

void APZ_Pizza::LowerQuality(float Amount)
{
	if (!HasAuthority() || Amount <= 0.f) return;

	Quality = FMath::Clamp(Quality - Amount, 0.f, 100.f);
	OnRep_Quality();
}

void APZ_Pizza::OnRep_Carried()
{
	ApplyCarriedPhysicsState();
}

void APZ_Pizza::OnRep_Quality()
{
	// TODO: on quality dropped (ex make the material lower, or pizza breaking)
}

void APZ_Pizza::ApplyCarriedPhysicsState()
{
	Mesh->SetSimulatePhysics(!bCarried);
	Mesh->SetCollisionEnabled(bCarried ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}
