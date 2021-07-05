#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Portable.generated.h"

class APortal;

UINTERFACE()
class PORTALS_API UPortable : public UInterface
{
	GENERATED_BODY()
	
};

class IPortable {

	GENERATED_BODY()

public:
	virtual void OnEnterPortalThreshold(APortal* portal);
	virtual void OnExitPortalThreshold();

	UFUNCTION(BlueprintImplementableEvent)
	TSubclassOf<AActor> GetCopyClass();
	//UFUNCTION(BlueprintImplementableEvent)
	//void SetMaterialProperties(bool isNearPortal, FVector normal);
	UFUNCTION(BlueprintImplementableEvent)
	void SetPortal(APortal* portal);
	UFUNCTION(BlueprintCallable)
	virtual AActor* GetCopy();

	AActor* Copy = nullptr;
	FVector LastPosition;
	bool LastInFront;
};