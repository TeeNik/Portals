#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PortalsGameMode.generated.h"

class USceneCaptureComponent2D;

UCLASS(minimalapi)
class APortalsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APortalsGameMode();

	UFUNCTION(BlueprintCallable)
	void SetCustomProjectionMatrix(USceneCaptureComponent2D* capture);

	UFUNCTION(BlueprintCallable)
	FMatrix GetCameraProjectionMatrix();

	UFUNCTION(BlueprintCallable)
	void UpdateCapture(USceneCaptureComponent2D* capture, UTextureRenderTarget2D* texture, AActor* portal, AActor* target);

};