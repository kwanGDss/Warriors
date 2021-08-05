// Copyright Epic Games, Inc. All Rights Reserved.

#include "WarriorsCharacter.h"

#include <string>

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "Math/Vector.h"

//////////////////////////////////////////////////////////////////////////
// AWarriorsCharacter

AWarriorsCharacter::AWarriorsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// 체력, 에너지, 기분 기정
	HealthValue = 1.0f;
	EnergyValue = 0.5f;
	MoodValue = 0.5f;

	//Set camera lag
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.0f;

	// Initialize Lock-on system
	bIsLockOnState = false;
	EnemyCharacter = nullptr;

	LockOnDirection = FVector::ZeroVector;
	LockOnRotation = FRotator::ZeroRotator;
	LockOnInterpolationRotation = FRotator::ZeroRotator;

	//Initialize Roll
	bIsRolling = false;

	RollStartedTime = 0.0f;
	RollDirection = FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X);

	//Initialize Running
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AWarriorsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("Running... : %f"), GetVelocity().Size());
	if (bIsLockOnState)
	{
		//check(EnemyCharacter);

		LockOnDirection = EnemyCharacter->GetActorLocation() - GetActorLocation();
		LockOnDirection = LockOnDirection.GetSafeNormal();

		LockOnRotation = UKismetMathLibrary::MakeRotFromX(LockOnDirection);
		LockOnRotation.Pitch -= 20.0f;

		LockOnInterpolationRotation = FMath::RInterpTo(GetController()->GetControlRotation(), LockOnRotation, DeltaTime, 5.0f);

		LockOnRotation.Pitch = 0.0f;

		if (!bIsRolling)
		{
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), LockOnRotation, DeltaTime, 3.0f));
		}

		GetController()->SetControlRotation(LockOnInterpolationRotation);
	}

	if (bIsRolling && BP_IsMoveSituation())
	{
		//	RollDirection = RollDirection.GetSafeNormal();
		//	FRotator RollRotation = UKismetMathLibrary::MakeRotFromX(RollDirection);

		//	if (RollDirection == FVector::ZeroVector)
		//	{
		//		RollDirection = GetActorForwardVector();
		//	}

		//	//SetActorRotation(FMath::RInterpTo(GetActorRotation(), RollRotation, DeltaTime, 10.0f));

		//	//SetActorLocation(FMath::VInterpTo(GetActorLocation(), RollDirection * 50.0f + GetActorLocation(), DeltaTime, 10.0f), true);

		if (GetWorld()->TimeSince(RollStartedTime) >= 0.7f)
		{
			bIsRolling = false;
			RollDirection = FVector::ZeroVector;
		}
	}

	if (bIsRunning)
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, 400.0f, DeltaTime, 10.0f);
	}
	else
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, 300.0f, DeltaTime, 10.0f);
	}

	if (GetVelocity().Size() > 10.f && bPressedShiftKey)
	{
		GetCharacterMovement()->MaxWalkSpeed = 720.0f;

		bIsRunning = true;
	}
}

void AWarriorsCharacter::SwitchLockOnState()
{
	bIsLockOnState = !bIsLockOnState;
}

void AWarriorsCharacter::LockOn()
{
	SwitchLockOnState();

	if (bIsLockOnState)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}

//void AWarriorsCharacter::Roll()
//{
//	if (!bIsRolling)
//	{
//		RollStartedTime = GetWorld()->GetTimeSeconds();
//		bIsRolling = true;
//	}
//}

void AWarriorsCharacter::Run()
{
	//UE_LOG(LogTemp, Warning, TEXT("Running... : %f"), GetVelocity().Size());
	if (!bIsLockOnState)
	{
		bPressedShiftKey = true;
	}
}

void AWarriorsCharacter::Walk()
{
	//UE_LOG(LogTemp, Warning, TEXT("Walking.."));
	if (!bIsLockOnState)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;

		bPressedShiftKey = false;
		bIsRunning = false;
	}
}

void AWarriorsCharacter::AddControllerPitchInput(float Val)
{
	if (!bIsRolling && !bIsLockOnState)
	{
		Super::AddControllerPitchInput(Val);
	}
}

void AWarriorsCharacter::AddControllerYawInput(float Val)
{
	if (!bIsRolling && !bIsLockOnState)
	{
		Super::AddControllerYawInput(Val);
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AWarriorsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AWarriorsCharacter::LockOn);
	//PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AWarriorsCharacter::Roll);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AWarriorsCharacter::Run);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AWarriorsCharacter::Walk);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWarriorsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWarriorsCharacter::MoveRight);

	PlayerInputComponent->BindAxis("RollForward", this, &AWarriorsCharacter::RollForward);
	PlayerInputComponent->BindAxis("RollRight", this, &AWarriorsCharacter::RollRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AWarriorsCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWarriorsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AWarriorsCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWarriorsCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AWarriorsCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AWarriorsCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AWarriorsCharacter::OnResetVR);
}

void AWarriorsCharacter::SetEnemyCharacter(TSubclassOf<AWarriorsCharacter> EnemyCharacterClass)
{
	EnemyCharacter = Cast<AWarriorsCharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), EnemyCharacterClass));
}


void AWarriorsCharacter::UpdateHealth(float HealthChange)
{
	HealthValue += HealthChange;
}

float AWarriorsCharacter::GetHealth()
{
	return HealthValue;
}

void AWarriorsCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AWarriorsCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AWarriorsCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AWarriorsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWarriorsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AWarriorsCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !bIsRolling && BP_IsMoveSituation() && !GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AWarriorsCharacter::RollForward(float Value)
{
	MoveForwardAxis = Value;

	if ((Controller != NULL) && (Value != 0.0f) && !bIsRolling && BP_IsMoveSituation())
	{
		if (Value > 0)
		{
			RollDirection = FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X);
		}
		else
		{
			RollDirection = -FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X);
		}
	}
}

void AWarriorsCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !bIsRolling && BP_IsMoveSituation() && !GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AWarriorsCharacter::RollRight(float Value)
{
	MoveRightAxis = Value;

	if ((Controller != NULL) && (Value != 0.0f) && !bIsRolling && BP_IsMoveSituation())
	{
		if (Value > 0)
		{
			RollDirection = FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y);
		}
		else
		{
			RollDirection = -FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y);
		}
	}
}
