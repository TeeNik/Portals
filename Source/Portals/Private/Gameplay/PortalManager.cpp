#include "Gameplay/PortalManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Gameplay/Portal.h"
#include "EngineUtils.h"

APortalManager::APortalManager()
{
    PrimaryActorTick.bCanEverTick = true;
    //SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    //SceneCapture->SetupAttachment(RootComponent);
    //
    //SceneCapture->bCaptureEveryFrame = false;
    //SceneCapture->bCaptureOnMovement = false;
    //SceneCapture->LODDistanceFactor = 3;
    //SceneCapture->TextureTarget = nullptr;
    //SceneCapture->bEnableClipPlane = true;
    //SceneCapture->bUseCustomProjectionMatrix = true;
    //SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
    //
    //FPostProcessSettings CaptureSettings;
    //
    //CaptureSettings.bOverride_AmbientOcclusionQuality = true;
    //CaptureSettings.bOverride_MotionBlurAmount = true;
    //CaptureSettings.bOverride_SceneFringeIntensity = true;
    //CaptureSettings.bOverride_GrainIntensity = true;
    //CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;
    //
    //CaptureSettings.AmbientOcclusionQuality = 0.0f;
    //CaptureSettings.MotionBlurAmount = 0.0f;
    //CaptureSettings.SceneFringeIntensity = 0.0f;
    //CaptureSettings.GrainIntensity = 0.0f;
    //CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;
    //
    //CaptureSettings.bOverride_ScreenPercentage = true;
    //CaptureSettings.ScreenPercentage = 100.0f;
    //
    //SceneCapture->PostProcessSettings = CaptureSettings;
}

void APortalManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (TActorIterator<APortal> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        APortal* Portal = *ActorItr;
        //Portal->ClearRTT();
        UpdateCapture(Portal);
    }
}

void APortalManager::BeginPlay()
{
    Super::BeginPlay();
    GeneratePortalTexture();
}

void APortalManager::RequestTeleportByPortal(APortal* Portal, AActor* TargetToTeleport)
{
}

void APortalManager::Init()
{
    GeneratePortalTexture();
}

void APortalManager::Update(float DeltaTime)
{

}

void APortalManager::UpdateCapture(APortal* Portal)
{
    /*
    USceneCaptureComponent2D* SceneCapture = Cast<USceneCaptureComponent2D>(SceneCaptureActor->GetComponentByClass(USceneCaptureComponent2D::StaticClass()));
    APortal* target = Portal->Target;
    USceneComponent* cameraTransform = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetTransformComponent();

    FVector newLocation = UTool::ConvertLocationToActorSpace(cameraTransform->GetComponentLocation(), Portal, target);
    SceneCapture->SetWorldLocation(newLocation);

    SceneCapture->bEnableClipPlane = true;
    SceneCapture->bUseCustomProjectionMatrix = true;
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

    FTransform CameraTransform = cameraTransform->GetComponentTransform();
    FTransform SourceTransform = Portal->GetActorTransform();
    FTransform TargetTransform = target->GetActorTransform();

    FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * CameraTransform.GetRotation();
    FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;
    SceneCapture->SetWorldRotation(NewWorldQuat);

    SceneCapture->ClipPlaneNormal = target->GetActorForwardVector();
    const bool IsPlayerInFront = target->IsPointInFrontOfPortal(SceneCapture->GetComponentLocation());
    if (IsPlayerInFront)
    {
        SceneCapture->ClipPlaneNormal *= -1.0;
    }
    SceneCapture->ClipPlaneBase = target->GetActorLocation() + SceneCapture->ClipPlaneNormal * ClipPlaneOffset;

    Portal->SetRTT(PortalTexture);
    SceneCapture->TextureTarget = PortalTexture;
    SceneCapture->bUseCustomProjectionMatrix = true;
    SceneCapture->CustomProjectionMatrix = GetCameraProjectionMatrix();

    SceneCapture->CaptureScene();
    */
}

void APortalManager::GeneratePortalTexture()
{
    int32 CurrentSizeX = 1920;
    int32 CurrentSizeY = 1080;

    GetWorld()->GetFirstPlayerController()->GetViewportSize(CurrentSizeX, CurrentSizeY);

    CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1920); //1920 / 1.5 = 1280
    CurrentSizeY = FMath::Clamp(int(CurrentSizeY / 1.7), 128, 1080);

    if (PortalTexture == nullptr)
    {
        PortalTexture = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), TEXT("PortalTexture"));
        check(PortalTexture);

        PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
        PortalTexture->Filter = TextureFilter::TF_Bilinear;
        PortalTexture->SizeX = CurrentSizeX;
        PortalTexture->SizeY = CurrentSizeY;
        PortalTexture->ClearColor = FLinearColor::Black;
        PortalTexture->TargetGamma = 2.2f;
        PortalTexture->bNeedsTwoCopies = false;
        PortalTexture->AddressX = TextureAddress::TA_Clamp;
        PortalTexture->AddressY = TextureAddress::TA_Clamp;
        PortalTexture->bAutoGenerateMips = false;
        PortalTexture->UpdateResource();
    }
}

FMatrix APortalManager::GetCameraProjectionMatrix()
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
