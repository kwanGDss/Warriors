// Fill out your copyright notice in the Description page of Project Settings.


#include "Warriors_GameMode.h"
#include "WarriorsCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWarriors_GameMode::AWarriors_GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}	
}