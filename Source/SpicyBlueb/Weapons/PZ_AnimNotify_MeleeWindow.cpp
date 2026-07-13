// Fill out your copyright notice in the Description page of Project Settings.


#include "PZ_AnimNotify_MeleeWindow.h"

#include "Components/SkeletalMeshComponent.h"
#include "SpicyBlueb/Core/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Core/Player/Components/PZ_EquipmentComponent.h"
#include "SpicyBlueb/Equippables/PZ_EquippableActor.h"

void UPZ_AnimNotify_MeleeWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	APZ_PlayerCharacter* Character = Cast<APZ_PlayerCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	UPZ_EquipmentComponent* EquipComp = Character->GetEquipmentComponent();
	if (!EquipComp) return;

	if (APZ_EquippableActor* EquippedActor = EquipComp->GetCurrentEquipped())
		EquippedActor->SetActiveVolumeEnabled(true);
}

void UPZ_AnimNotify_MeleeWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
	APZ_PlayerCharacter* Character = Cast<APZ_PlayerCharacter>(MeshComp->GetOwner());
	if (!Character) return;

	UPZ_EquipmentComponent* EquipComp = Character->GetEquipmentComponent();
	if (!EquipComp) return;

	if (APZ_EquippableActor* EquippedActor = EquipComp->GetCurrentEquipped())
		EquippedActor->SetActiveVolumeEnabled(false);
}
