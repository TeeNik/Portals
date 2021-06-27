#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

class APortal;
class USceneCaptureComponent2D;

UCLASS()
class PORTALS_API APortalManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APortalManager();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void RequestTeleportByPortal(APortal* Portal, AActor* TargetToTeleport);

	void Init();
	void Update(float DeltaTime);
	void UpdateCapture(APortal* Portal);

private:
	void GeneratePortalTexture();

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;
	UPROPERTY(transient)
	UTextureRenderTarget2D* PortalTexture = nullptr;
	UPROPERTY()
	APlayerController* ControllerOwner;

	int32 PreviousScreenSizeX = 0;
	int32 PreviousScreenSizeY = 0;

	float UpdateDelay = 1.1f;
};
