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

UCLASS()
class SPICYBLUEB_API APZ_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APZ_PlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
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
	
	// Input handlers
	void Move(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	
	void AddInputMapping();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebug = false;
	
private:
	FVector DesiredFacing = FVector::ForwardVector;
	bool bUsingGamepadAim = false;
	
	// Helpers
	void UpdateMouseFacing();
	void ApplyFacing(float DeltaTime);
};
