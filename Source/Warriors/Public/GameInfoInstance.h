// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <iostream>
#include <string>
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInfoInstance.generated.h"

using namespace std;

/**
 * 
 */
UCLASS()
class WARRIORS_API UGameInfoInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UGameInfoInstance();
	~UGameInfoInstance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	FString IPAddress;

	// ü��
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	float HealthValue;

	// ������
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	float EnergyValue;
};
