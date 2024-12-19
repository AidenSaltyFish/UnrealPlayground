// Copyright Epic Games, Inc. All Rights Reserved.

#include "YoshiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AYoshiCharacter

AYoshiCharacter::AYoshiCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AYoshiCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AYoshiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AYoshiCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AYoshiCharacter::Look);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Ongoing, this, &AYoshiCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AYoshiCharacter::StopSprint);
		
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Ongoing, this, &AYoshiCharacter::Crouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AYoshiCharacter::StopCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AYoshiCharacter::Move(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsCrouching())
	{
		return;
	}
	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AYoshiCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AYoshiCharacter::Sprint(const FInputActionValue& Value)
{
	float SprintValue = Value.Get<float>();
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	GetCharacterMovement()->MaxAcceleration = 3000.f;
}

void AYoshiCharacter::StopSprint(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MaxAcceleration = 1500.f;
}

void AYoshiCharacter::Crouch(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	float crouchHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
	if (!GetCharacterMovement()->IsCrouching())
	{
		ACharacter::Crouch(false);

		FVector camPos = CameraBoom->GetComponentLocation();
	
		float z = FMath::FInterpTo(
			camPos.Z,
			camPos.Z - crouchHeight,
			GetWorld()->GetDeltaSeconds(),
			1.0f);
	
		CameraBoom->SetWorldLocation(FVector(camPos.X, camPos.Y, z));
	}
	
	// if (GetCharacterMovement()->IsCrouching())
	// {
	// 	ACharacter::UnCrouch();
	// 	crouchHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
	// }
	// else
	// {
	// 	ACharacter::Crouch(false);		
	// 	// GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	// 	crouchHeight = -GetCharacterMovement()->GetCrouchedHalfHeight();
	// }
}

void AYoshiCharacter::StopCrouch(const FInputActionValue& Value)
{
	float crouchHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
	if (GetCharacterMovement()->IsCrouching())
	{
		ACharacter::UnCrouch();

		// FVector camPos = CameraBoom->GetComponentLocation();
		//
		// float z = FMath::FInterpTo(
		// 	camPos.Z,
		// 	camPos.Z + crouchHeight,
		// 	GetWorld()->GetDeltaSeconds(),
		// 	5.0f);
		//
		// CameraBoom->SetWorldLocation(FVector(camPos.X, camPos.Y, z));
	}
}
