#include "PZ_GameModeBase.h"

#include "SpicyBlueb/Player/PZ_PlayerCharacter.h"
#include "SpicyBlueb/Player/PZ_PlayerController.h"

APZ_GameModeBase::APZ_GameModeBase()
{
	// TODO [Elias Brown]: Hook these up
	DefaultPawnClass = APZ_PlayerCharacter::StaticClass();
	PlayerControllerClass = APZ_PlayerController::StaticClass();
}
