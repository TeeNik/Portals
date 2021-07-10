#include "Gameplay/PortableComponent.h"
#include "Gameplay/Portal.h"

UPortableComponent::UPortableComponent()
{
}

void UPortableComponent::OnEnterPortalThreshold(APortal* portal)
{
	if(CopyClass)
	{
		if (Copy != nullptr)
		{
			Copy->SetActorHiddenInGame(false);
		}
		else
		{
			Copy = GetWorld()->SpawnActor<AActor>(CopyClass);
		}
	}

	Portal = portal;
}

void UPortableComponent::OnExitPortalThreshold()
{
	if (Copy != nullptr)
	{
		Copy->SetActorHiddenInGame(true);
	}
	Portal = nullptr;
}

void UPortableComponent::Teleport(const FVector& newLocation, const FQuat& newRotation)
{
	FHitResult HitResult;
	GetOwner()->SetActorLocation(newLocation, false, &HitResult, ETeleportType::TeleportPhysics);
	GetOwner()->SetActorRotation(newRotation);
	LastPosition = newLocation;
}
