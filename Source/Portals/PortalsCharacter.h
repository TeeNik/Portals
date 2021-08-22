#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PortalsCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class ATelekinesisActor;

UCLASS(config=Game)
class APortalsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APortalsCharacter();

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

protected:
	virtual void Tick(float DeltaSeconds) override;

	void OnFire();
	void MoveForward(float Val);
	void MoveRight(float Val);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* TelekinesisSocket;

private:
	UPROPERTY(Transient)
	ATelekinesisActor* TelekinesisTarget;

	bool IsTargetCaptured;

};