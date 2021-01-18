// Copyright Epic Games, Inc. All Rights Reserved.

#include "WarriorsGameMode.h"
#include "WarriorsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWarriorsGameMode::AWarriorsGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
