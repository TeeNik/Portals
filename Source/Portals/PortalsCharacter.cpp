#include "PortalsCharacter.h"
#include "PortalsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "Gameplay/TelekinesisActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

APortalsCharacter::APortalsCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	TelekinesisSocket = CreateDefaultSubobject<USceneComponent>(TEXT("TelekinesisSocket"));
	TelekinesisSocket->SetupAttachment(FirstPersonCameraComponent);
}

void APortalsCharacter::Tick(float DeltaSeconds)
{
	if (IsTargetCaptured)
	{
		TelekinesisTarget->ReachTarget(TelekinesisSocket->GetComponentLocation());
	}
	else
	{
		FHitResult hit;
		FVector start = FirstPersonCameraComponent->GetComponentLocation();
		FVector end = start + FirstPersonCameraComponent->GetForwardVector() * 10000;
		const bool result = GetWorld()->LineTraceSingleByChannel(hit, start, end, ECC_Visibility);
		if (result)
		{
			ATelekinesisActor* target = Cast<ATelekinesisActor>(hit.Actor);
			if (TelekinesisTarget)
			{
				TelekinesisTarget->SetHighlight(false);
			}
			if (target)
			{
				TelekinesisTarget = target;
				TelekinesisTarget->SetHighlight(true);
			}
		}
		else
		{
			if (IsValid(TelekinesisTarget))
			{
				TelekinesisTarget->SetHighlight(false);
				TelekinesisTarget = nullptr;
			}
		}
	}

	Super::Tick(DeltaSeconds);
}

void APortalsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APortalsCharacter::OnFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &APortalsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APortalsCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APortalsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APortalsCharacter::LookUpAtRate);
}

void APortalsCharacter::OnFire()
{
	if (TelekinesisTarget)
	{
		if (IsTargetCaptured)
		{
			TelekinesisTarget->Push(GetFirstPersonCameraComponent()->GetForwardVector());
			TelekinesisTarget = nullptr;
			IsTargetCaptured = false;
		}
		else
		{
			IsTargetCaptured = true;
			TelekinesisTarget->OnCapture();
		}
	}

	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void APortalsCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APortalsCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APortalsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APortalsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}