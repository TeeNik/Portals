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
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void RequestTeleportByPortal(APortal* Portal, AActor* TargetToTeleport);

	void Init();
	void Update(float DeltaTime);
	void UpdateCapture(APortal* Portal);

	UPROPERTY(EditAnywhere)
	float ClipPlaneOffset = -1.5f;

	UPROPERTY(EditAnywhere)
		AActor* SceneCaptureActor;

	UPROPERTY(EditAnywhere)
		UTextureRenderTarget2D* PortalTexture = nullptr;

protected:
	virtual void BeginPlay() override;

private:
	void GeneratePortalTexture();
	FMatrix GetCameraProjectionMatrix();

	//UPROPERTY()
	//USceneCaptureComponent2D* SceneCapture;

	UPROPERTY()
	APlayerController* ControllerOwner;

	int32 PreviousScreenSizeX = 0;
	int32 PreviousScreenSizeY = 0;

	float UpdateDelay = 1.1f;
};