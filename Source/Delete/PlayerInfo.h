// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "PlayerInfo.generated.h"

/**
 * 
 */
/*class WARRIORS_API PlayerInfo
{
public:
	PlayerInfo();
	~PlayerInfo();
};*/

USTRUCT(Atomic, BlueprintType)
struct FPlayerInfo
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MyPlayerName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MyPlayerCharacter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MyPlayerCharacterImage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MyPlayerStatus;
};