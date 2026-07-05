#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PZ_Pizza.generated.h"

class UStaticMeshComponent;

UCLASS()
class SPICYBLUEB_API APZ_Pizza : public AActor
{
	GENERATED_BODY()

public:
	APZ_Pizza();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing=OnRep_Quality, EditAnywhere, BlueprintReadOnly, Category="Pizz")
	float Quality = 100.f;

	UFUNCTION(BlueprintCallable, Category = "Pizza")
	void PickUp(USceneComponent* AttachTo, FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Pizza")
	void ThrowOrDrop(const FVector& Impulse);
	
	UFUNCTION(BlueprintCallable, Category = "Pizza")
	void LowerQuality(float Amount);

	UFUNCTION(BlueprintPure, Category = "Pizza")
	bool IsCarried() const { return bCarried; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pizza")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(ReplicatedUsing = OnRep_Carried, BlueprintReadOnly, Category = "Pizza")
	bool bCarried = false;

	UFUNCTION()
	void OnRep_Carried();

	UFUNCTION()
	void OnRep_Quality();

private:
	void ApplyCarriedPhysicsState();
};