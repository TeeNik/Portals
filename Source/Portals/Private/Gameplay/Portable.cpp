#include "Gameplay/Portable.h"

void IPortable::OnEnterPortalThreshold()
{
	AActor* actor = Cast<AActor>(this);
	if (IsValid(actor))
	{
		if(IsValid(Copy))
		{
			Copy->SetHidden(false);
		} 
		else
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Template = actor;
			Copy = actor->GetWorld()->SpawnActor<AActor>(actor->GetClass());
		}
	}
}

void IPortable::OnExitPortalThreshold()
{
	if(IsValid(Copy))
	{
		Copy->SetHidden(true);
	}
}