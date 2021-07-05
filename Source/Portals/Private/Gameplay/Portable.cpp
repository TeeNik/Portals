#include "Gameplay/Portable.h"
#include "Gameplay/Portal.h"

void IPortable::OnEnterPortalThreshold(APortal* portal)
{
	AActor* actor = Cast<AActor>(this);
	if (IsValid(actor))
	{
		if(Copy != nullptr)
		{
			Copy->SetHidden(false);
		} 
		else
		{
			Copy = actor->GetWorld()->SpawnActor<AActor>(Execute_GetCopyClass(actor));
		}
		bool isInFront = portal->IsPointInFrontOfPortal(actor->GetActorLocation());
		FVector normal = portal->GetActorForwardVector();
		if(isInFront)
		{
			normal *= -1;
		}
		Execute_SetPortal(actor, portal);
		//Execute_SetMaterialProperties(actor, true, normal);
	}
}

void IPortable::OnExitPortalThreshold()
{
	if(Copy != nullptr)
	{
		Copy->SetHidden(true);
	}
	Execute_SetPortal(Cast<AActor>(this), nullptr);
	//Execute_SetMaterialProperties(Cast<AActor>(this), false, FVector::ZeroVector);
}

AActor* IPortable::GetCopy()
{
	return Copy;
}