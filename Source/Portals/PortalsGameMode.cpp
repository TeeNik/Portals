#include "PortalsGameMode.h"
#include "PortalsHUD.h"
#include "PortalsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Gameplay/Tool.h"

APortalsGameMode::APortalsGameMode() : Super()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = APortalsHUD::StaticClass();
}

void APortalsGameMode::SetCustomProjectionMatrix(USceneCaptureComponent2D* capture)
{
	ULocalPlayer* localPlayer = GetWorld()->GetFirstPlayerController()->GetLocalPlayer();
	if(IsValid(localPlayer) && IsValid(capture))
	{
		FSceneViewProjectionData playerProjectionData;
		localPlayer->GetProjectionData(localPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, playerProjectionData);
		capture->bUseCustomProjectionMatrix = true;
		capture->CustomProjectionMatrix = playerProjectionData.ProjectionMatrix;
	}
}

FMatrix APortalsGameMode::GetCameraProjectionMatrix()
{
	FMatrix ProjectionMatrix;
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstPlayerController()->GetLocalPlayer();

	if (LocalPlayer != nullptr)
	{
		FSceneViewProjectionData PlayerProjectionData;
		LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, PlayerProjectionData);
		ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
	}

	return ProjectionMatrix;
}

void APortalsGameMode::UpdateCapture(USceneCaptureComponent2D* capture, UTextureRenderTarget2D* texture, AActor* portal, AActor* target)
{
	USceneComponent* cameraTransform = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetTransformComponent();

	FVector newLocation = UTool::ConvertLocationToActorSpace(cameraTransform->GetComponentLocation(), portal, target);
	capture->GetOwner()->SetActorLocation(newLocation);

	FTransform CameraTransform = cameraTransform->GetComponentTransform();
	FTransform SourceTransform = portal->GetActorTransform();
	FTransform TargetTransform = target->GetActorTransform();

	FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * CameraTransform.GetRotation();
	FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;
	capture->GetOwner()->SetActorRotation(NewWorldQuat);

	capture->ClipPlaneNormal = target->GetActorForwardVector();
	capture->ClipPlaneBase = target->GetActorLocation() + capture->ClipPlaneNormal * -1.5f;

	capture->TextureTarget = texture;
	capture->bUseCustomProjectionMatrix = true;
	capture->CustomProjectionMatrix = GetCameraProjectionMatrix();

	capture->CaptureScene();
}
