#pragma once

#include "CoreMinimal.h"
#include "Tool.generated.h"

UCLASS()
class PORTALS_API UTool : public UObject
{
	GENERATED_BODY()

public:
	UTool();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FVector ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FRotator ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
};