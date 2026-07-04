#include "PZ_GameModeBase.h"
#include "Kismet/GameplayStatics.h"

APZ_GameModeBase::APZ_GameModeBase()
{
}

void APZ_GameModeBase::BeginPlay()
{
	Super::BeginPlay();
	//UGameplayStatics::CreatePlayer(this, 1, true);
}
