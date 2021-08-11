#include "Gameplay/PortableComponent.h"
#include "Gameplay/Portal.h"
#include "Chaos/Collision/NarrowPhase.h"

UPortableComponent::UPortableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
}

void UPortableComponent::OnEnterPortalThreshold(APortal* portal)
{
	if(ShouldCreateCopy && !IsCopy)
	{
		if (Copy != nullptr)
		{
			Copy->SetActorHiddenInGame(false);
		}
		else
		{
			Copy = GetWorld()->SpawnActor<AActor>(GetOwner()->GetClass());
			Copy->SetActorLabel(Copy->GetActorLabel() + TEXT("_Copy"));
			UPortableComponent* portable = Cast<UPortableComponent>(Copy->GetComponentByClass(UPortableComponent::StaticClass()));
			portable->IsCopy = true;
			portable->GetOwnerMesh()->SetEnableGravity(false);
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

void UPortableComponent::UpdateSliceMaterial()
{
	if (!IsCopy)
	{
		UStaticMeshComponent* mesh = GetOwnerMesh();
		const bool isPortalValid = IsValid(Portal);
		mesh->SetScalarParameterValueOnMaterials(TEXT("UseMask"), isPortalValid);

		if (isPortalValid)
		{
			mesh->SetVectorParameterValueOnMaterials(TEXT("Pos"), Portal->GetActorLocation());
			FVector normal = Portal->GetActorForwardVector();
			if (Portal->IsPointInFrontOfPortal(mesh->GetComponentLocation()))
			{
				normal *= -1.0f;
			}
			mesh->SetVectorParameterValueOnMaterials(TEXT("Normal"), normal);

			if (IsValid(Copy))
			{
				UPortableComponent* copyPortable = Cast<UPortableComponent>(Copy->GetComponentByClass(UPortableComponent::StaticClass()));
				UStaticMeshComponent* copyMesh = copyPortable->GetOwnerMesh();
				copyMesh->SetScalarParameterValueOnMaterials(TEXT("UseMask"), 1.0f);
				copyMesh->SetVectorParameterValueOnMaterials(TEXT("Pos"), Portal->Target->GetActorLocation());
				FVector copyNormal = Portal->Target->GetActorForwardVector();
				if (!Portal->Target->IsPointInFrontOfPortal(mesh->GetComponentLocation()))
				{
					copyNormal *= -1.0f;
				}
				copyMesh->SetVectorParameterValueOnMaterials(TEXT("Normal"), copyNormal);
			}
		}

		if (IsValid(Copy) && Portal)
		{
			AActor* owner = GetOwner();
			Copy->SetActorLocation(Portal->ConvertLocationToActorSpace(owner->GetActorLocation(), Portal->GetTransform(), Portal->Target->GetTransform()));
			Copy->SetActorRotation(Portal->ConvertRotationToActorSpace(owner->GetActorRotation(), Portal->GetTransform(), Portal->Target->GetTransform()));
		}
	}
}

void UPortableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	UpdateSliceMaterial();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UStaticMeshComponent* UPortableComponent::GetOwnerMesh()
{
	if(CachedMesh == nullptr)
	{
		CachedMesh = Cast<UStaticMeshComponent>(GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	}
	return CachedMesh;
}
