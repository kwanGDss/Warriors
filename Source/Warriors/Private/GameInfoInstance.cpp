// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInfoInstance.h"



UGameInfoInstance::UGameInfoInstance()
{
    char* tmp = TCHAR_TO_ANSI(*IPAddress);
}

UGameInfoInstance::~UGameInfoInstance()
{

}