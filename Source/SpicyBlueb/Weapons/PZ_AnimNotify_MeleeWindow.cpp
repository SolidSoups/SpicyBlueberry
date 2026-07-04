// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_AnimNotify_MeleeWindow.h"

#include "Components/SkeletalMeshComponent.h"
#include "SpicyBlueb/Player/PZ_PlayerCharacter.h"

void UPZ_AnimNotify_MeleeWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (APZ_PlayerCharacter* Character = Cast<APZ_PlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->SetWeaponCollisionEnabled(true);
	}
}

void UPZ_AnimNotify_MeleeWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (APZ_PlayerCharacter* Character = Cast<APZ_PlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->SetWeaponCollisionEnabled(false);
	}
}
