#include "Gameplay/Tool.h"

UTool::UTool()
{
}

FVector UTool::ConvertLocationToActorSpace(FVector Location, AActor* Reference, AActor* Target)
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

FRotator UTool::ConvertRotationToActorSpace(FRotator Rotation, AActor* Reference, AActor* Target)
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

bool UTool::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point);

	//If < 0 means we are behind the Plane
    UE_LOG(LogTemp, Log, TEXT("IsPointInFrontOfPortal %d"), PortalDot >= 0);
	return PortalDot >= 0;
}