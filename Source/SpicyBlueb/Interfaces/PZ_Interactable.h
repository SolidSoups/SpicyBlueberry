#pragma once
#include "UObject/Interface.h"
#include "PZ_Interactable.generated.h"

class APZ_PlayerCharacter;

UINTERFACE(MinimalAPI)
class UPZ_Interactable : public UInterface
{
	GENERATED_BODY()	
};

class SPICYBLUEB_API IPZ_Interactable
{
	GENERATED_BODY()
	
public:
	virtual void OnInteract(APZ_PlayerCharacter* Interactor) = 0;
};
