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

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnEnterPortalThreshold(APortal* portal);
	virtual void OnExitPortalThreshold();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> CopyClass;
	
	UPROPERTY(Transient)
	AActor* Copy = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	APortal* Portal = nullptr;
	
	FVector LastPosition;
	bool LastInFront;

protected:
	virtual void BeginPlay() override;
		
};
