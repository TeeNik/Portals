#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortableComponent.generated.h"

class APortal;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTALS_API UPortableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPortableComponent();

	virtual void OnEnterPortalThreshold(APortal* portal);
	virtual void OnExitPortalThreshold();
	virtual void Teleport(const FVector& newLocation, const FQuat& newRotation);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> CopyClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient)
	AActor* Copy = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	APortal* Portal = nullptr;
	
	FVector LastPosition;
	bool LastInFront;

};
