#include "Gameplay/PortalPlayer.h"
#include "SceneView.h"

UPortalPlayer::UPortalPlayer()
{
	UseCameraCut = false;
}

FSceneView* UPortalPlayer::CalcSceneView(FSceneViewFamily* ViewFamily, FVector& OutViewLocation, FRotator& OutViewRotation, FViewport* Viewport, FViewElementDrawer* ViewDrawer, EStereoscopicPass StereoPass)
{
    FSceneView* view = Super::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);
    if (UseCameraCut)
    {
        view->bCameraCut = true;
    }
    return view;
}

void UPortalPlayer::CameraCut()
{
	UseCameraCut = true;
}

FMatrix UPortalPlayer::GetCameraProjectionMatrix()
{
    FMatrix projectionMatrix;
    FSceneViewProjectionData playerProjectionData;
    GetProjectionData(ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, playerProjectionData);
    projectionMatrix = playerProjectionData.ProjectionMatrix;
    return projectionMatrix;
}