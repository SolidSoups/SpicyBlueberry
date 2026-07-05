// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "PZ_PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class APZ_Shovel;
class APZ_Pizza;
class APZ_Restaurant;

UCLASS()
class SPICYBLUEB_API APZ_PlayerCharacter : public ACharacter
{
private:
	GENERATED_BODY()

public:
	APZ_PlayerCharacter();
	

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackMontage();

	UFUNCTION(BlueprintPure, Category = "PZ|Pizza")
	APZ_Pizza* GetCarriedPizza() const { return CarriedPizza; }

	void CarryPizza(APZ_Pizza* Pizza);
	void ClearCarriedPizza() { CarriedPizza = nullptr; }
	void SetOverlappingRestaurant(APZ_Restaurant* R) { OverlappingRestaurant = R; }

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackMontage();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetWeaponCollisionEnabled(bool IsEnabled);
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetFacingDirection() const ;

protected:
	// Input handlers
	void Move(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void DoAttack();
	void Interact();
	
	void AddInputMapping();
	
	
	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	
	// Inputs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;
	
	// Feel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel|Camera")
	float CameraHeight = 1500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel|Camera")
	float CameraPitch = -55.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel|Camera")
	float CameraYaw = 45.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel|Aim")
	float AimRotationSpeed = 720.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel|Aim")
	float GamepadAimDeadzone = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebug = false;
	
	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<APZ_Shovel> ShovelClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<APZ_Shovel> EquippedShovel;
	
	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
	
private:
	void SpawnAndAttachShovel();	
	
	// Helpers
	void UpdateMouseFacing();
	void ApplyFacing(float DeltaTime);
	
	UPROPERTY(Replicated)
	float RepFacingYaw = 0.f;
	
	// Attack trigger is incremented by server when a character attacks. When a client or proxy
	// receives a notify, they compare against their copy to detect new attacks:
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_AttackTrigger)
	uint8 RepAttackTrigger = 0;
	
	UFUNCTION()
	void OnRep_AttackTrigger();

	float LastAppliedYaw = 0.f;

	const FName HandSocketName = TEXT("HandGrip_R");
	const FName PizzaSocketName = TEXT("PizzaHold");
	
	UPROPERTY(Replicated)
	TObjectPtr<APZ_Pizza> CarriedPizza = nullptr;

	UPROPERTY()
	TObjectPtr<APZ_Restaurant> OverlappingRestaurant = nullptr;

	UFUNCTION(Server, Reliable)
	void Server_Interact();
	
	UFUNCTION(Server, Unreliable)
	void Server_SetFacingYaw(float NewYaw);
	
	UFUNCTION(Server, Reliable)
	void Server_DoAttack();
	
	FVector DesiredFacing = FVector::ForwardVector;
	bool bUsingGamepadAim = false;
};
