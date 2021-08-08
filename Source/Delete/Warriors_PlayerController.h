// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ClientSocket.h"
#include "WarriorsCharacter.h"
#include "Warriors_PlayerController.generated.h"

UCLASS()
class WARRIORS_API AWarriors_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWarriors_PlayerController();
	~AWarriors_PlayerController();

	// ���� ���̵� ��ȭ
	UFUNCTION(BlueprintPure, Category = "Properties")
	int GetSessionId();
	
	// HUD ȭ�鿡�� �� ���� Ŭ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties", Meta = (BlueprintProtect = "true"))
	TSubclassOf<class UUserWidget> HUDWidgetClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Properties", Meta = (BlueprintProtect = "true"))
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	// HUD ��ü
	UPROPERTY()
	class UUserWidget* CurrentWidget;

	UPROPERTY()
	class UUserWidget* GameOverWidget;

	// ������ų �ٸ� ĳ����
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> WhoToSpawn;

	// �ı��� �� ��ƼŬ
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* DestroyEmiiter;

	// Ÿ���� �� ��ƼŬ
	UPROPERTY(EditAnywhere, Category = "Spawning")
	UParticleSystem* HitEmiiter;

	// ���Ͽ��� �ٸ� ĳ���� Ÿ�� ���� ����
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	//void HitCharacter(const int& SessionId);
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	

	// ���Ǿ��̵� ��Ī�Ǵ� ���� ��ȯ
	//AActor* FindActorBySessionId(TArray<AActor*> ActorArray, const int& SessionId);	

	// �������κ��� ���� ���� ����
	void RecvWorldInfo(cCharactersInfo * ci);

	// �� �÷��̾� ������Ʈ
	//void RecvNewPlayer(cCharacter * NewPlayer);

private:
	AClientSocket *		Socket;			// ������ ������ ����
	bool				bIsConnected;	// ������ ���� ����
	int					SessionId;		// ĳ������ ���� ���� ���̵� (������)
	cCharactersInfo *	CharactersInfo;	// �ٸ� ĳ������ ����

	void SendPlayerInfo();		// �÷��̾� ��ġ �۽�
	bool UpdateWorldInfo();		// ���� ����ȭ
	void UpdatePlayerInfo(const cCharacter & info);		// �÷��̾� ����ȭ	

	FTimerHandle SendPlayerInfoHandle;	// ����ȭ Ÿ�̸� �ڵ鷯
	
	// �� �÷��̾� ����
	int	nPlayers;
	bool bNewPlayerEntered;
	cCharacter * NewPlayer;
	void UpdateNewPlayer();
};
