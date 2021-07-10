#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "PortalPlayer.generated.h"

UCLASS()
class PORTALS_API UPortalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UPortalPlayer();

	virtual FSceneView* CalcSceneView(class FSceneViewFamily* ViewFamily, FVector& OutViewLocation, FRotator& OutViewRotation, FViewport* Viewport, class FViewElementDrawer* ViewDrawer, EStereoscopicPass StereoPass) override;
	void CameraCut();
	FMatrix GetCameraProjectionMatrix();

private:
	bool UseCameraCut;

};
