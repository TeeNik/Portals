#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class PORTALS_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Portal")
	bool IsActive;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void ClearRTT();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void SetRTT(UTexture* RenderTexture);
	UFUNCTION(BlueprintNativeEvent, Category = "Portal")
	void ForceTick();
	UFUNCTION(BlueprintPure, Category = "Portal")
	AActor* GetTarget();
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SetTarget(AActor* InTarget);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* PortalRootComponent;

private:
	UPROPERTY(Transient)
	AActor* Target;
	FVector LastPosition;
	bool LastInFront;
};
