#include "Gameplay/Portal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Tool.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/BoxComponent.h"
#include "Gameplay/Portable.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
	IsActive = false;

    PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = PortalRootComponent;

    PortalView = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalView"));
    PortalView->SetupAttachment(RootComponent);
    
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);
    InteractionBox->OnComponentEndOverlap.AddDynamic(this, &APortal::OnOverlapEnd);
        
    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    SceneCapture->SetupAttachment(RootComponent);

    SceneCapture->bCaptureEveryFrame = false;
    SceneCapture->bCaptureOnMovement = false;
    SceneCapture->LODDistanceFactor = 3;
    SceneCapture->TextureTarget = nullptr;
    SceneCapture->bEnableClipPlane = true;
    SceneCapture->bUseCustomProjectionMatrix = true;
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;

    FPostProcessSettings CaptureSettings;

    CaptureSettings.bOverride_AmbientOcclusionQuality = true;
    CaptureSettings.bOverride_MotionBlurAmount = true;
    CaptureSettings.bOverride_SceneFringeIntensity = true;
    CaptureSettings.bOverride_GrainIntensity = true;
    CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

    CaptureSettings.AmbientOcclusionQuality = 0.0f;
    CaptureSettings.MotionBlurAmount = 0.0f;
    CaptureSettings.SceneFringeIntensity = 0.0f;
    CaptureSettings.GrainIntensity = 0.0f;
    CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;

    CaptureSettings.bOverride_ScreenPercentage = true;
    CaptureSettings.ScreenPercentage = 100.0f;

    SceneCapture->PostProcessSettings = CaptureSettings;
}

void APortal::BeginPlay()
{
    Super::BeginPlay();
    GeneratePortalTexture();
    SceneCapture->HiddenComponents.Add(PortalView);
}

void APortal::Tick(float DeltaTime)
{
    Update(DeltaTime);

    if(PortableTargets.Num() > 0)
    {
        for(int i = 0; i < PortableTargets.Num(); ++i)
        {
            AActor* actor = Cast<AActor>(PortableTargets[i]);
            if (actor)
            {
                if (IsPointCrossingPortal(PortableTargets[i]))
                {
                    TickInProgress = true;
                    TeleportActor(actor);
					PortableTargets[i]->OnExitPortalThreshold();
                    PortableTargets.RemoveAt(i);
                    --i;
                    TickInProgress = false;
                }
            }
        }
    }
}

void APortal::Update(float DeltaTime)
{
    if(IsValid(Target))
    {
        UpdateCapture();
    }
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    IPortable* portable = Cast<IPortable>(OtherActor);
    if(portable != nullptr)
    {
        if(!PortableTargets.Contains(portable))
        {
            UE_LOG(LogTemp, Log, TEXT("OnOverlapBegin"));
            FVector point = OtherActor->GetActorLocation();
            portable->LastInFront = IsPointInFrontOfPortal(point);
            portable->LastPosition = point;
            portable->OnEnterPortalThreshold();
            PortableTargets.Add(portable);
        }
    }
}

void APortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
    IPortable* portable = Cast<IPortable>(OtherActor);
    if (portable != nullptr)
    {
        AActor* actor = Cast<AActor>(portable);
        if(actor && PortableTargets.Contains(portable) && !TickInProgress)
        {
            UE_LOG(LogTemp, Log, TEXT("OnOverlapEnd"));
            portable->OnExitPortalThreshold();
            PortableTargets.Remove(portable);
        }
    }
}

void APortal::GeneratePortalTexture()
{
    int32 CurrentSizeX = 1920;
    int32 CurrentSizeY = 1080;

    //GetWorld()->GetFirstPlayerController()->GetViewportSize(CurrentSizeX, CurrentSizeY);    //CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1920); //1920 / 1.5 = 128/0
    //CurrentSizeY = FMath::Clamp(int(CurrentSizeY / 1.7), 128, 1080);

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

FMatrix APortal::GetCameraProjectionMatrix()
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

void APortal::UpdateCapture()
{
    USceneComponent* cameraTransformComponent = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetTransformComponent();
    USceneCaptureComponent2D* targetCapture = Target->SceneCapture;

    //FVector newLocation = UTool::ConvertLocationToActorSpace(cameraTransform->GetComponentLocation(), this, Target);
    //targetCapture->SetWorldLocation(newLocation);

    FTransform CameraTransform = cameraTransformComponent->GetComponentTransform();
    FTransform SourceTransform = GetActorTransform();
    FTransform TargetTransform = Target->GetActorTransform();

    FVector newLocation = TargetTransform.TransformPosition(SourceTransform.InverseTransformPosition(CameraTransform.GetLocation()));
    targetCapture->SetWorldLocation(newLocation);

    FQuat newRotation = TargetTransform.TransformRotation(SourceTransform.InverseTransformRotation(CameraTransform.GetRotation()));
    targetCapture->SetWorldRotation(newRotation);

    targetCapture->ClipPlaneNormal = Target->GetActorForwardVector();
    const bool IsPlayerInFront = Target->IsPointInFrontOfPortal(targetCapture->GetComponentLocation());
    if (IsPlayerInFront)
    {
        targetCapture->ClipPlaneNormal *= -1.0;
    }
    targetCapture->ClipPlaneBase = Target->GetActorLocation() + targetCapture->ClipPlaneNormal * ClipPlaneOffset;

    SetRTT(PortalTexture);
    targetCapture->TextureTarget = PortalTexture;
    targetCapture->bUseCustomProjectionMatrix = true;
    targetCapture->CustomProjectionMatrix = GetCameraProjectionMatrix();

    targetCapture->CaptureScene();
}

bool APortal::IsPointInFrontOfPortal(FVector Point) const
{
    FPlane PortalPlane = FPlane(GetActorLocation(), GetActorForwardVector());
    float PortalDot = PortalPlane.PlaneDot(Point);
    //If < 0 means we are behind the Plane
    return (PortalDot >= 0);
}

bool APortal::IsPointCrossingPortal(IPortable* Portable)
{
    FVector IntersectionPoint;
    FVector PortalLocation = GetActorLocation();
    FVector PortalNormal = GetActorForwardVector();

    AActor* actor = Cast<AActor>(Portable);
    FVector Point = actor->GetActorLocation();

    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    bool IsCrossing = false;
    bool IsInFront =  IsPointInFrontOfPortal(Point);

    bool IsIntersect = FMath::SegmentPlaneIntersection(Portable->LastPosition, Point, PortalPlane, IntersectionPoint);

    // Did we intersect the portal since last Location ?
    // If yes, check the direction : crossing forward means we were in front and now at the back
    // If we crossed backward, ignore it (similar to Prey 2006)
    if (IsIntersect && IsInFront != Portable->LastInFront)
    {
        IsCrossing = true;
    }

    // Store values for next check
    Portable->LastInFront = IsInFront;
    Portable->LastPosition = Point;

    return IsCrossing;
}

void APortal::TeleportActor(AActor* ActorToTeleport)
{

    if (ActorToTeleport == nullptr || Target == nullptr)
    {
        return;
    }

    FVector savedVelocity = FVector::ZeroVector;
    ACharacter* character = nullptr;

    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        character = Cast<ACharacter>(ActorToTeleport);
        savedVelocity = character->GetCharacterMovement()->Velocity;
    }

    FHitResult HitResult;
    FVector NewLocation = UTool::ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), this, Target);
    ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);
    FRotator NewRotation = UTool::ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), this, Target);
    ActorToTeleport->SetActorRotation(NewRotation);

    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (pc != nullptr)
        {
            NewRotation = UTool::ConvertRotationToActorSpace(pc->GetControlRotation(), this, Target);
            pc->SetControlRotation(NewRotation);
        }

        //Reapply Velocity (Need to reorient direction into local space of Portal)
        {
            FVector Dots;
            Dots.X = FVector::DotProduct(savedVelocity, GetActorForwardVector());
            Dots.Y = FVector::DotProduct(savedVelocity, GetActorRightVector());
            Dots.Z = FVector::DotProduct(savedVelocity, GetActorUpVector());

            FVector NewVelocity = Dots.X * Target->GetActorForwardVector()
                + Dots.Y * Target->GetActorRightVector()
                + Dots.Z * Target->GetActorUpVector();

            character->GetCharacterMovement()->Velocity = NewVelocity;
        }
    }

    //Cleanup Teleport
    Cast<IPortable>(ActorToTeleport)->LastPosition = NewLocation;
}


void APortal::ForceTick_Implementation()
{
}

void APortal::SetRTT_Implementation(UTexture* RenderTexture)
{
}

void APortal::ClearRTT_Implementation()
{
}