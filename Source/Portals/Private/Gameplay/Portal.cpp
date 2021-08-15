#include "Gameplay/Portal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/BoxComponent.h"
#include "Gameplay/PortableComponent.h"
#include "Gameplay/PortalPlayer.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

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
                    OnTeleportUsed(false);
                    Target->OnTeleportUsed(true);

                    FVector prevPos = actor->GetActorLocation();
                    FRotator prevRot = actor->GetActorRotation();

                	TeleportActor(actor);
                    if(portable->Copy)
                    {
                        portable->Copy->SetActorLocationAndRotation(prevPos, prevRot);
                    }

                    Target->UpdateCapture();
                    PortableTargets.RemoveAt(i);
                    --i;

                    Target->AddPortableTarget(portable);
                    portable->UpdateSliceMaterial();

                    TickInProgress = false;
                } 
            }
        }
    }

    Super::Tick(DeltaTime);
}

void APortal::AddPortableTarget(UPortableComponent* portable)
{
    if (!PortableTargets.Contains(portable) && !portable->IsCopy)
    {
        FVector point = portable->GetOwner()->GetActorLocation();
        portable->LastInFront = IsPointInFrontOfPortal(point);
        portable->LastPosition = point;
        portable->OnEnterPortalThreshold(this);
        PortableTargets.Add(portable);
    }
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UActorComponent* component = OtherActor->GetComponentByClass(UPortableComponent::StaticClass());
    if(component)
    {
        UPortableComponent* portable = Cast<UPortableComponent>(component);
        AddPortableTarget(portable);
    }
}

void APortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,	int32 OtherBodyIndex)
{
    UActorComponent* component = OtherActor->GetComponentByClass(UPortableComponent::StaticClass());
    if (component)
    {
        UPortableComponent* portable = Cast<UPortableComponent>(component);
	    if(PortableTargets.Contains(portable) && !portable->IsCopy && !TickInProgress)
	    {
            portable->OnExitPortalThreshold();
            PortableTargets.Remove(portable);
	    }
    }
}

void APortal::GeneratePortalTexture()
{
    int32 CurrentSizeX = 1920;
    int32 CurrentSizeY = 1080;

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

void APortal::UpdateCapture()
{
    USceneComponent* cameraTransformComponent = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetTransformComponent();
    USceneCaptureComponent2D* targetCapture = Target->SceneCapture;

    FTransform CameraTransform = cameraTransformComponent->GetComponentTransform();
    FTransform SourceTransform = GetActorTransform();
    FTransform TargetTransform = Target->GetActorTransform();

    FVector newLocation = TargetTransform.TransformPosition(SourceTransform.InverseTransformPosition(CameraTransform.GetLocation()));
    FQuat newRotation = TargetTransform.TransformRotation(SourceTransform.InverseTransformRotation(CameraTransform.GetRotation()));
    
	targetCapture->SetWorldLocation(newLocation);
	targetCapture->SetWorldRotation(newRotation);

    SetRTT(PortalTexture);
    targetCapture->TextureTarget = PortalTexture;
    targetCapture->CustomProjectionMatrix = Cast<UPortalPlayer>(GetWorld()->GetFirstPlayerController()->GetLocalPlayer())->GetCameraProjectionMatrix();
    
    for (int i = RecursionAmount; i >= 0; i--)
    {
        FVector recursiveCamLoc = newLocation;
        FRotator recursiveCamRot = newRotation.Rotator();
        for (int p = 0; p < i; p++)
        {
            recursiveCamLoc = ConvertLocationToActorSpace(recursiveCamLoc, SourceTransform, TargetTransform);
            recursiveCamRot = ConvertRotationToActorSpace(recursiveCamRot, SourceTransform, TargetTransform).Rotator();
        }
        targetCapture->SetWorldLocationAndRotation(recursiveCamLoc, recursiveCamRot);

        if (i == RecursionAmount)
        {
            PortalView->SetVisibility(false);
        }

        bool isInsidePortal = UKismetMathLibrary::IsPointInBox(CameraTransform.GetLocation(), InteractionBox->GetComponentLocation(), InteractionBox->Bounds.BoxExtent * 2);
        targetCapture->bEnableClipPlane = !isInsidePortal;
        if (!isInsidePortal)
        {
            targetCapture->ClipPlaneNormal = Target->GetActorForwardVector();
            const bool IsPlayerInFront = Target->IsPointInFrontOfPortal(recursiveCamLoc);
            if (IsPlayerInFront)
            {
                targetCapture->ClipPlaneNormal *= -1.0;
            }
            targetCapture->ClipPlaneBase = Target->GetActorLocation() + targetCapture->ClipPlaneNormal * ClipPlaneOffset;
        }


        targetCapture->CaptureScene();

        if (i == RecursionAmount)
        {
            PortalView->SetVisibility(true);
        }
    }
}

FVector APortal::ConvertLocationToActorSpace(const FVector& actorLocation, const FTransform& source, const FTransform& target)
{
    return target.TransformPosition(source.InverseTransformPosition(actorLocation));
}

FQuat APortal::ConvertRotationToActorSpace(const FRotator& actorRotation, const FTransform& source, const FTransform& target)
{
    return target.TransformRotation(source.InverseTransformRotation(actorRotation.Quaternion()));
}

FVector APortal::ConvertDirectionToTarget(FVector direction)
{
    FVector flippedVel;
    flippedVel.X = FVector::DotProduct(direction, GetActorForwardVector());
    flippedVel.Y = FVector::DotProduct(direction, GetActorRightVector());
    flippedVel.Z = FVector::DotProduct(direction, GetActorUpVector());
    FVector newVelocity = flippedVel.X * Target->GetActorForwardVector()
        + flippedVel.Y * Target->GetActorRightVector()
        + flippedVel.Z * Target->GetActorUpVector();

    return newVelocity;
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

    UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(ActorToTeleport->GetRootComponent());
    FVector newLinearVelocity = ConvertDirectionToTarget(prim->GetPhysicsLinearVelocity());
    FVector newAngularVelocity = ConvertDirectionToTarget(prim->GetPhysicsAngularVelocityInDegrees());
    prim->SetPhysicsLinearVelocity(newLinearVelocity);
    prim->SetPhysicsAngularVelocityInDegrees(newAngularVelocity);

    FHitResult HitResult;
    FVector NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), GetTransform(), Target->GetTransform());
    ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);
    FQuat NewRotation = ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), GetTransform(), Target->GetTransform());
    ActorToTeleport->SetActorRotation(NewRotation);


    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (pc != nullptr)
        {
            NewRotation = Target->GetTransform().TransformRotation(GetTransform().InverseTransformRotation(pc->GetControlRotation().Quaternion()));
            pc->SetControlRotation(NewRotation.Rotator());
        }

        ACharacter* character = Cast<ACharacter>(ActorToTeleport);
        FVector savedVelocity = character->GetCharacterMovement()->Velocity;

        FVector Dots;
        Dots.X = FVector::DotProduct(savedVelocity, GetActorForwardVector());
        Dots.Y = FVector::DotProduct(savedVelocity, GetActorRightVector());
        Dots.Z = FVector::DotProduct(savedVelocity, GetActorUpVector());
        
        FVector NewVelocity = Dots.X * Target->GetActorForwardVector()
            + Dots.Y * Target->GetActorRightVector()
            + Dots.Z * Target->GetActorUpVector();
        
        character->GetCharacterMovement()->Velocity = NewVelocity;
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