#include "Gameplay/PortableComponent.h"
#include "Gameplay/Portal.h"

UPortableComponent::UPortableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPortableComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPortableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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