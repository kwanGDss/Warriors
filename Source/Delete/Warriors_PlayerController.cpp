// Fill out your copyright notice in the Description page of Project Settings.


#include "Warriors_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include <string>
#include "TimerManager.h"
#include <vector>
#include <algorithm>

AWarriors_PlayerController::AWarriors_PlayerController()
{
	
}

AWarriors_PlayerController::~AWarriors_PlayerController()
{
    SessionId = FMath::RandRange(0, 10000);

	// 서버와 연결
	//Socket = AClientSocket::GetSingleton();
	//Socket->InitSocket();
	//bIsConnected = Socket->Connect("127.0.0.1", 8000);
	if (bIsConnected)
	{
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
		//Socket->SetPlayerController(this);
	}

	//bIsChatNeedUpdate = false;
	bNewPlayerEntered = false;

	nPlayers = -1;

	PrimaryActorTick.bCanEverTick = true;
}

int AWarriors_PlayerController::GetSessionId()
{
    return SessionId;
}

/*void AWarriors_PlayerController::HitCharacter(const int& SessionId)
{
	UE_LOG(LogClass, Log, TEXT("Player Hit Called %d"), SessionId);

	//Socket->HitPlayer(SessionId);
}*/

void AWarriors_PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsConnected) return;

	// FIXME
	// SendPlayerInfo();

	// 새로운 플레이어 입장
	if (bNewPlayerEntered)
		UpdateNewPlayer();
	// 월드 동기화
	UpdateWorldInfo();
}

void AWarriors_PlayerController::BeginPlay()
{
	UE_LOG(LogClass, Log, TEXT("BeginPlay Start"));
	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

	// 캐릭터 등록
	/*auto Player = Cast<AWarriorsCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
		return;

	Player->SessionId = SessionId;

	auto MyLocation = Player->GetActorLocation();
	auto MyRotation = Player->GetActorRotation();*/

	/*cCharacter Character;
	// 위치
	Character.SessionId = SessionId;
	Character.X = MyLocation.X;
	Character.Y = MyLocation.Y;
	Character.Z = MyLocation.Z;
	// 회전
	Character.Yaw = MyRotation.Yaw;
	Character.Pitch = MyRotation.Pitch;
	Character.Roll = MyRotation.Roll;
	// 속도
	Character.VX = 0;
	Character.VY = 0;
	Character.VZ = 0;
	// 속성
	Character.IsAlive = Player->IsAlive;
	Character.HealthValue = Player->HealthValue;
	Character.IsAttacking = Player->IsAttacking;

	Socket->EnrollPlayer(Character);*/

	// Recv 스레드 시작
	//Socket->StartListen();
	UE_LOG(LogClass, Log, TEXT("BeginPlay End"));

	// FIXME 
	// 플레이어 동기화 시작	
	GetWorldTimerManager().SetTimer(SendPlayerInfoHandle, this, &AWarriors_PlayerController::SendPlayerInfo, 0.016f, true);
	//GetWorldTimerManager().SetTimer(SendPlayerInfoHandle, &SendPlayerInfo, 0.016f, true);
}

void AWarriors_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Socket->LogoutPlayer(SessionId);
	//Socket->CloseSocket();
	//Socket->StopListen();
}

/*AActor* AWarriors_PlayerController::FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId)
{
   for (const auto& Actor : ActorArray)
	{
		AWarriorsCharacter * warriors_c = Cast<AWarriorsCharacter>(Actor);
		//if (warriors_c && warriors_c->SessionId == SessionId)
			return Actor;
	}
	return nullptr;
}*/

void AWarriors_PlayerController::RecvWorldInfo(cCharactersInfo* ci)
{
	//if (ci_ != nullptr)
	{
		//CharactersInfo = ci_;
	}
}

/*void AWarriors_PlayerController::RecvNewPlayer(cCharacter* NewPlayer)
{
	//if (NewPlayer_ != nullptr)
	{
		bNewPlayerEntered = true;
		//NewPlayer = NewPlayer_;
	}
}*/

void AWarriors_PlayerController::SendPlayerInfo()
{
	/*auto Player = Cast<AWarriorsCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player)
		return;

	// 플레이어의 위치를 가져옴	
	const auto & Location = Player->GetActorLocation();
	const auto & Rotation = Player->GetActorRotation();
	const auto & Velocity = Player->GetVelocity();

	cCharacter Character;
	Character.SessionId = SessionId;

	Character.X = Location.X;
	Character.Y = Location.Y;
	Character.Z = Location.Z;

	Character.Yaw = Rotation.Yaw;
	Character.Pitch = Rotation.Pitch;
	Character.Roll = Rotation.Roll;

	Character.VX = Velocity.X;
	Character.VY = Velocity.Y;
	Character.VZ = Velocity.Z;

	Character.IsAlive = Player->IsAlive;
	Character.HealthValue = Player->HealthValue;
	Character.IsAttacking = Player->IsAttacking;

	Socket->SendPlayer(Character);*/
}

bool AWarriors_PlayerController::UpdateWorldInfo()
{
    /*UWorld* const world = GetWorld();
	if (world == nullptr)
		return false;

	if (CharactersInfo == nullptr)
		return false;

	// 플레이어 업데이트
	UpdatePlayerInfo(CharactersInfo->players[SessionId]);

	// 다른 플레이어 업데이트
	TArray<AActor*> SpawnedCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWarriorsCharacter::StaticClass(), SpawnedCharacters);

	if (nPlayers == -1)
	{	
		for (auto & player : CharactersInfo->players)
		{			
			if (player.first == SessionId || !player.second.IsAlive)
				continue;

			FVector SpawnLocation;
			SpawnLocation.X = player.second.X;
			SpawnLocation.Y = player.second.Y;
			SpawnLocation.Z = player.second.Z;

			FRotator SpawnRotation;
			SpawnRotation.Yaw = player.second.Yaw;
			SpawnRotation.Pitch = player.second.Pitch;
			SpawnRotation.Roll = player.second.Roll;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;
			SpawnParams.Name = FName(*FString(to_string(player.second.SessionId).c_str()));

			AWarriorsCharacter* SpawnCharacter = world->SpawnActor<AWarriorsCharacter>(WhoToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
			SpawnCharacter->SpawnDefaultController();

			SpawnCharacter->SessionId = player.second.SessionId;
			SpawnCharacter->HealthValue = player.second.HealthValue;
			SpawnCharacter->IsAlive = player.second.IsAlive;
			SpawnCharacter->IsAttacking = player.second.IsAttacking;
		}

		nPlayers = CharactersInfo->players.size();
	} 
	else
	{
		for (auto& Character : SpawnedCharacters)
		{
			AWarriorsCharacter * OtherPlayer = Cast<AWarriorsCharacter>(Character);

			if (!OtherPlayer || OtherPlayer->SessionId == -1 || OtherPlayer->SessionId == SessionId)
			{
				continue;
			}

			cCharacter * info = &CharactersInfo->players[OtherPlayer->SessionId];

			if (info->IsAlive)
			{
				if (OtherPlayer->HealthValue != info->HealthValue)
				{
					UE_LOG(LogClass, Log, TEXT("other player damaged."));
					// 피격 파티클 소환
					FTransform transform(OtherPlayer->GetActorLocation());
					UGameplayStatics::SpawnEmitterAtLocation(
						world, HitEmiiter, transform, true
					);
					// 피격 애니메이션 플레이
					OtherPlayer->PlayDamagedAnimation();
					OtherPlayer->HealthValue = info->HealthValue;
				}

				// 공격중일때 타격 애니메이션 플레이
				if (info->IsAttacking)
				{
					UE_LOG(LogClass, Log, TEXT("other player hit."));
					OtherPlayer->PlayHitAnimation();
				}

				FVector CharacterLocation;
				CharacterLocation.X = info->X;
				CharacterLocation.Y = info->Y;
				CharacterLocation.Z = info->Z;

				FRotator CharacterRotation;
				CharacterRotation.Yaw = info->Yaw;
				CharacterRotation.Pitch = info->Pitch;
				CharacterRotation.Roll = info->Roll;

				FVector CharacterVelocity;
				CharacterVelocity.X = info->VX;
				CharacterVelocity.Y = info->VY;
				CharacterVelocity.Z = info->VZ;

				OtherPlayer->AddMovementInput(CharacterVelocity);
				OtherPlayer->SetActorRotation(CharacterRotation);
				OtherPlayer->SetActorLocation(CharacterLocation);
			}
			else
			{
				UE_LOG(LogClass, Log, TEXT("other player dead."));
				FTransform transform(Character->GetActorLocation());
				UGameplayStatics::SpawnEmitterAtLocation(
					world, DestroyEmiiter, transform, true
				);
				Character->Destroy();
			}
		}
	}
	*/
	return true;
}

void AWarriors_PlayerController::UpdatePlayerInfo(const cCharacter& info)
{
	/*auto Player = Cast<AWarriorsCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
		return;

	UWorld* const world = GetWorld();
	if (!world)
		return;

	if (!info.IsAlive)
	{
		UE_LOG(LogClass, Log, TEXT("Player Die"));
		FTransform transform(Player->GetActorLocation());
		UGameplayStatics::SpawnEmitterAtLocation(
			world, DestroyEmiiter, transform, true
		);
		Player->Destroy();

		CurrentWidget->RemoveFromParent();
		GameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
		if (GameOverWidget != nullptr)
		{
			GameOverWidget->AddToViewport();
		}
	}
	else
	{
		// 캐릭터 속성 업데이트
		if (Player->HealthValue != info.HealthValue)
		{
			UE_LOG(LogClass, Log, TEXT("Player damaged"));
			// 피격 파티클 스폰
			FTransform transform(Player->GetActorLocation());
			UGameplayStatics::SpawnEmitterAtLocation(
				world, HitEmiiter, transform, true
			);
			// 피격 애니메이션 스폰
			//Player->PlayDamagedAnimation();
			Player->HealthValue = info.HealthValue;
		}
	}*/

}

void AWarriors_PlayerController::UpdateNewPlayer()
{
	UWorld* const world = GetWorld();

	// 새로운 플레이어가 자기 자신이면 무시
	if (NewPlayer->SessionId == SessionId)
	{
		bNewPlayerEntered = false;
		NewPlayer = nullptr;
		return;
	}
		
	// 새로운 플레이어를 필드에 스폰
	/*FVector SpawnLocation;
	SpawnLocation.X = NewPlayer->X;
	SpawnLocation.Y = NewPlayer->Y;
	SpawnLocation.Z = NewPlayer->Z;

	FRotator SpawnRotation;
	SpawnRotation.Yaw = NewPlayer->Yaw;
	SpawnRotation.Pitch = NewPlayer->Pitch;
	SpawnRotation.Roll = NewPlayer->Roll;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	//SpawnParams.Instigator = Instigator;
	SpawnParams.Name = FName(*FString(to_string(NewPlayer->SessionId).c_str()));

	AWarriorsCharacter* SpawnCharacter = world->SpawnActor<AWarriorsCharacter>(WhoToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	SpawnCharacter->SpawnDefaultController();
	//SpawnCharacter->SessionId = NewPlayer->SessionId;
	SpawnCharacter->HealthValue = NewPlayer->HealthValue;
	*/
	// 필드의 플레이어 정보에 추가
	if (CharactersInfo != nullptr)
	{
		cCharacter player;
		player.SessionId = NewPlayer->SessionId;
		player.X = NewPlayer->X;
		player.Y = NewPlayer->Y;
		player.Z = NewPlayer->Z;

		player.Yaw = NewPlayer->Yaw;
		player.Pitch = NewPlayer->Pitch;
		player.Roll = NewPlayer->Roll;

		player.VX = NewPlayer->VX;
		player.VY = NewPlayer->VY;
		player.VZ = NewPlayer->VZ;

		player.IsAlive = NewPlayer->IsAlive;
		player.HealthValue = NewPlayer->HealthValue;
		player.IsAttacking = NewPlayer->IsAttacking;

		CharactersInfo->players[NewPlayer->SessionId] = player;
	}

	UE_LOG(LogClass, Log, TEXT("other player spawned."));

	bNewPlayerEntered = false;
	NewPlayer = nullptr;
}
