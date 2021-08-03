#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TelekinesisActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class PORTALS_API ATelekinesisActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATelekinesisActor();

	void SetHighlight(bool isHighlighted);
	void OnCapture();
	void ReachTarget(FVector target);
	void Push(FVector direction);

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly)
	float ReachMaxSpeed = 1000.0f;
	UPROPERTY(EditDefaultsOnly)
	float PushForce = 1000.0f;

};
