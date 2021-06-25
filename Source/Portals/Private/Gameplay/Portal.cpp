#include "Gameplay/Portal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
	IsActive = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->Mobility = EComponentMobility::Static;

    PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
    PortalRootComponent->SetupAttachment(GetRootComponent());
    PortalRootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    PortalRootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    PortalRootComponent->Mobility = EComponentMobility::Movable;

}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AActor* APortal::GetTarget()
{
    return Target;
}

void APortal::SetTarget(AActor* InTarget)
{
    Target = InTarget;
}

bool APortal::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    float PortalDot = PortalPlane.PlaneDot(Point);

    //If < 0 means we are behind the Plane
    return (PortalDot >= 0);
}

bool APortal::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
    FVector IntersectionPoint;
    FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
    float PortalDot = PortalPlane.PlaneDot(Point);
    bool IsCrossing = false;
    bool IsInFront = PortalDot >= 0;

    bool IsIntersect = FMath::SegmentPlaneIntersection(LastPosition, Point, PortalPlane, IntersectionPoint);

    // Did we intersect the portal since last Location ?
    // If yes, check the direction : crossing forward means we were in front and now at the back
    // If we crossed backward, ignore it (similar to Prey 2006)
    if (IsIntersect && !IsInFront && LastInFront)
    {
        IsCrossing = true;
    }

    // Store values for next check
    LastInFront = IsInFront;
    LastPosition = Point;

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
    FVector NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), this, Target);
    ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);
    FRotator NewRotation = ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), this, Target);
    ActorToTeleport->SetActorRotation(NewRotation);

    if (ActorToTeleport->IsA(ACharacter::StaticClass()))
    {
        APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (pc != nullptr)
        {
            NewRotation = ConvertRotationToActorSpace(pc->GetControlRotation(), this, Target);
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
    LastPosition = NewLocation;
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

FVector APortal::ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* Target)
{
    if (Reference == nullptr || Target == nullptr)
    {
        return FVector::ZeroVector;
    }

    FVector Direction = Location - Reference->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();

    FVector Dots;
    Dots.X = FVector::DotProduct(Direction, Reference->GetActorForwardVector());
    Dots.Y = FVector::DotProduct(Direction, Reference->GetActorRightVector());
    Dots.Z = FVector::DotProduct(Direction, Reference->GetActorUpVector());

    FVector NewDirection = Dots.X * Target->GetActorForwardVector()
        + Dots.Y * Target->GetActorRightVector()
        + Dots.Z * Target->GetActorUpVector();

    return TargetLocation + NewDirection;
}

FRotator APortal::ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* Target)
{
    if (Reference == nullptr || Target == nullptr)
    {
        return FRotator::ZeroRotator;
    }

    FTransform SourceTransform = Reference->GetActorTransform();
    FTransform TargetTransform = Target->GetActorTransform();
    FQuat QuatRotation = FQuat(Rotation);

    FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * QuatRotation;
    FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;

    return NewWorldQuat.Rotator();
}