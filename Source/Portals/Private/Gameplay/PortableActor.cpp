#include "Gameplay/PortableActor.h"

APortableActor::APortableActor()
{
}

AActor* APortableActor::GetCopy()
{
	return Copy;
}
