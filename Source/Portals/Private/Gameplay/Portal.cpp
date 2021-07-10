#include "Gameplay/Portal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/BoxComponent.h"
#include "Gameplay/PortableComponent.h"
#include "Gameplay/PortalPlayer.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
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
    Super::Tick(DeltaTime);

    UpdateCapture();

    if(PortableTargets.Num() > 0)
    {
        for(int i = 0; i < PortableTargets.Num(); ++i)
        {
            UPortableComponent* portable = PortableTargets[i];
            AActor* actor = portable->GetOwner();
            if (actor)
            {
                if (IsPointCrossingPortal(portable))
                {
                    TickInProgress = true;
                    Cast<UPortalPlayer>(GetWorld()->GetFirstPlayerController()->GetLocalPlayer())->CameraCut();
                    TeleportActor(actor);
                    portable->OnExitPortalThreshold();
                    Target->UpdateCapture();
                    PortableTargets.RemoveAt(i);
                    --i;
                    TickInProgress = false;
                } 
            	else
                {
                    if(IsValid(portable->Copy))
                    {
                        UE_LOG(LogTemp, Log, TEXT("Update copy location"));
                        portable->Copy->SetActorLocation(ConvertLocationToActorSpace(actor->GetTransform(), GetTransform(), Target->GetTransform()));
                        portable->Copy->SetActorRotation(ConvertRotationToActorSpace(actor->GetTransform(), GetTransform(), Target->GetTransform()));
                    }
                }
            }
        }
    }
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UActorComponent* component = OtherActor->GetComponentByClass(UPortableComponent::StaticClass());
    if(component)
    {
        UPortableComponent* portable = Cast<UPortableComponent>(component);
        if (!PortableTargets.Contains(portable))
        {
            UE_LOG(LogTemp, Log, TEXT("OnOverlapBegin"));
            FVector point = OtherActor->GetActorLocation();
            portable->LastInFront = IsPointInFrontOfPortal(point);
            portable->LastPosition = point;
            portable->OnEnterPortalThreshold(this);
            PortableTargets.Add(portable);
        }
    }
}

void APortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
    UActorComponent* component = OtherActor->GetComponentByClass(UPortableComponent::StaticClass());
    if (component)
    {
        UPortableComponent* portable = Cast<UPortableComponent>(component);
	    if(PortableTargets.Contains(portable) && !TickInProgress)
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

    //GetWorld()->GetFirstPlayerController()->GetViewportSize(CurrentSizeX, CurrentSizeY);
    //CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1920); //1920 / 1.5 = 128/0
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
    UE_LOG(LogTemp, Log, TEXT("GetCameraProjectionMatrix"));

    if (LocalPlayer != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("LocalPlayer != nullptr"));
        FSceneViewProjectionData PlayerProjectionData;
        LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, PlayerProjectionData);
        ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
    }

    return ProjectionMatrix;
}

void APortal::CustomTick(float DeltaTime)
{
    SetMaterialScale(0.0);
    ClearRTT();
    UpdateCapture();

    if (PortableTargets.Num() > 0)
    {
        for (int i = 0; i < PortableTargets.Num(); ++i)
        {
            UPortableComponent* portable = PortableTargets[i];
            AActor* actor = portable->GetOwner();
            if (actor)
            {
                if (IsPointCrossingPortal(portable))
                {
                    TickInProgress = true;
                    Cast<UPortalPlayer>(GetWorld()->GetFirstPlayerController()->GetLocalPlayer())->CameraCut();
                    TeleportActor(actor);
                    portable->OnExitPortalThreshold();
                    SetMaterialScale(1.0);
                    Target->SetMaterialScale(1.0);
                    Target->UpdateCapture();
                    PortableTargets.RemoveAt(i);
                    --i;
                    TickInProgress = false;
                }
                else
                {
                    if (IsValid(portable->Copy))
                    {
                        UE_LOG(LogTemp, Log, TEXT("Update copy location"));
                        portable->Copy->SetActorLocation(ConvertLocationToActorSpace(actor->GetTransform(), GetTransform(), Target->GetTransform()));
                        portable->Copy->SetActorRotation(ConvertRotationToActorSpace(actor->GetTransform(), GetTransform(), Target->GetTransform()));
                    }
                }
            }
        }
    }
}

void APortal::UpdateCapture()
{
    USceneComponent* cameraTransformComponent = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetTransformComponent();
    USceneCaptureComponent2D* targetCapture = Target->SceneCapture;

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
    targetCapture->CustomProjectionMatrix = Cast<UPortalPlayer>(GetWorld()->GetFirstPlayerController()->GetLocalPlayer())->GetCameraProjectionMatrix();
    targetCapture->CaptureScene();
}

FVector APortal::ConvertLocationToActorSpace(const FTransform& actor, const FTransform& source, const FTransform& target)
{
    return target.TransformPosition(source.InverseTransformPosition(actor.GetLocation()));
}

FQuat APortal::ConvertRotationToActorSpace(const FTransform& actor, const FTransform& source, const FTransform& target)
{
    return target.TransformRotation(source.InverseTransformRotation(actor.GetRotation()));
}

bool APortal::IsPointInFrontOfPortal(FVector Point) const
{
    FPlane PortalPlane = FPlane(GetActorLocation(), GetActorForwardVector());
    float PortalDot = PortalPlane.PlaneDot(Point);
    //If < 0 means we are behind the Plane
    return (PortalDot >= 0);
}

bool APortal::IsPointCrossingPortal(UPortableComponent* Portable)
{
    FVector IntersectionPoint;
    FVector PortalLocation = GetActorLocation();
    FVector PortalNormal = GetActorForwardVector();

    AActor* actor = Portable->GetOwner();
    FVector Point = actor->GetActorLocation();

    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    bool IsCrossing = false;
    bool IsInFront =  IsPointInFrontOfPortal(Point);

    bool IsIntersect = FMath::SegmentPlaneIntersection(Portable->LastPosition, Point, PortalPlane, IntersectionPoint);

    if (IsIntersect && IsInFront != Portable->LastInFront)
    {
        IsCrossing = true;
    }

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
    FVector NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetTransform(), GetTransform(), Target->GetTransform());
    ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);
    FQuat NewRotation = ConvertRotationToActorSpace(ActorToTeleport->GetTransform(), GetTransform(), Target->GetTransform());
    ActorToTeleport->SetActorRotation(NewRotation);

    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (pc != nullptr)
        {
            NewRotation = Target->GetTransform().TransformRotation(GetTransform().InverseTransformRotation(pc->GetControlRotation().Quaternion()));
            pc->SetControlRotation(NewRotation.Rotator());
        }


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

    UPortableComponent* portable = Cast<UPortableComponent>(ActorToTeleport->GetComponentByClass(UPortableComponent::StaticClass()));
    portable->LastPosition = NewLocation;
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