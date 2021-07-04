#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Portable.generated.h"

UINTERFACE()
class PORTALS_API UPortable : public UInterface
{
	GENERATED_BODY()
	
};

class IPortable {

	GENERATED_BODY()

public:
	virtual void OnEnterPortalThreshold();
	virtual void OnExitPortalThreshold();

	AActor* Copy = nullptr;
	FVector LastPosition;
	bool LastInFront;
};