#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortableComponent.generated.h"

class APortal;
class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTALS_API UPortableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPortableComponent();

	virtual void OnEnterPortalThreshold(APortal* portal);
	virtual void OnExitPortalThreshold();
	virtual void Teleport(const FVector& newLocation, const FQuat& newRotation);

	void UpdateSliceMaterial();

	UPROPERTY(EditDefaultsOnly)
	bool ShouldCreateCopy = true;
	UPROPERTY(BlueprintReadWrite, Transient)
	AActor* Copy = nullptr;
	UPROPERTY(BlueprintReadWrite, Transient)
	APortal* Portal = nullptr;
	UPROPERTY(BlueprintReadOnly)
	bool IsCopy = false;
	
	FVector LastPosition;
	bool LastInFront;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	UStaticMeshComponent* CachedMesh = nullptr;

	UStaticMeshComponent* GetOwnerMesh();
};
