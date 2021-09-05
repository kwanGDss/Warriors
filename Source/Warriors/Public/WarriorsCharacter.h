// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClientSocket.h"
#include "WarriorsCharacter.generated.h"

UCLASS(config=Game)
class AWarriorsCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:
	AWarriorsCharacter();

	virtual void Tick(float DeltaTime) override;

	//void SwitchLockOnState();

	//UFUNCTION(BlueprintCallable)
	//void LockOn();

	//void Roll();

	//void Run();
	//void Walk();

	//virtual void AddControllerPitchInput(float Val) override;
	//virtual void AddControllerYawInput(float Val) override;

	//UFUNCTION(BlueprintImplementableEvent)
	//bool BP_IsMoveSituation();

	///** Returns CameraBoom subobject **/
	//FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	///** Returns FollowCamera subobject **/
	//FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsDamaged;//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bHitTheEnemy;//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bCanParry;//

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bCanParriedByEnemy; // server 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bBeParriedByEnemy; // server 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsGuard;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bHitGuardByEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bIsDead; //

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Properties")
	FString CharacterName; //delete
	
protected:
	///** Resets HMD orientation in VR. */
	//void OnResetVR();

	///** Called for forwards/backward input */
	//void MoveForward(float Value);
	//void RollForward(float Value);

	///** Called for side to side input */
	//void MoveRight(float Value);
	//void RollRight(float Value);

	///** 
	// * Called via input to turn at a given rate. 
	// * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	// */
	//void TurnAtRate(float Rate);

	///**
	// * Called via input to turn look up/down at a given rate. 
	// * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	// */
	//void LookUpAtRate(float Rate);

	///** Handler for when a touch input begins. */
	//void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	///** Handler for when a touch input stops. */
	//void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	//// APawn interface
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//// End of APawn interface

	//UFUNCTION(BlueprintCallable)
	//virtual void SetEnemyCharacter(TSubclassOf<AWarriorsCharacter> EnemyCharacter);

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

private:
	///** Camera boom positioning the camera behind the character */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	//class USpringArmComponent* CameraBoom;

	///** Follow camera */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	//class UCameraComponent* FollowCamera;

	///** Lock-on system */
	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = LockOn, Meta = (AllowPrivateAccess = true))
	//bool bIsLockOnState;

	//UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = LockOn, Meta = (AllowPrivateAccess = true))
	//class AWarriorsCharacter* EnemyCharacter;
	//
	//FVector RollDirection;
	//FVector LockOnDirection;
	//FRotator LockOnRotation;
	//FRotator LockOnInterpolationRotation;

	///** Rolling */
	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Roll, Meta = (AllowPrivateAccess = true))
	//bool bIsRolling;

	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Roll, Meta = (AllowPrivateAccess = true))
	//float RollStartedTime;

	///** Running */
	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Run, Meta = (AllowPrivateAccess = true))
	//bool bIsRunning;

	//UPROPERTY(VisibleInstanceOnly, Transient, Category = Run, Meta = (AllowPrivateAccess = true))
	//bool bPressedShiftKey;

	///** Move Axis */
	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Move, Meta = (AllowPrivateAccess = true))
	//float MoveForwardAxis;

	//UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Move, Meta = (AllowPrivateAccess = true))
	//float MoveRightAxis;
};