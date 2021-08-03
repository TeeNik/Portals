#include "Gameplay/TelekinesisActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ATelekinesisActor::ATelekinesisActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->CustomDepthStencilValue = 253;
}

void ATelekinesisActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATelekinesisActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATelekinesisActor::SetHighlight(bool isHighlighted)
{
	Mesh->SetRenderCustomDepth(isHighlighted);
}

void ATelekinesisActor::OnCapture()
{
	SetHighlight(false);
	Mesh->SetEnableGravity(false);
	Mesh->SetAngularDamping(2.0f);
	Mesh->SetLinearDamping(15.0f);
}

void ATelekinesisActor::ReachTarget(FVector target)
{
	Mesh->SetEnableGravity(false);

	float massClamped = UKismetMathLibrary::MapRangeClamped(Mesh->GetMass(), 50.0f, 700.0f, 5.0f, 1.0f);
	FVector distClamped = UKismetMathLibrary::ClampVectorSize(target - GetActorLocation(), 0.0f, ReachMaxSpeed);
	FVector impulse = distClamped * massClamped;
	Mesh->AddImpulse(impulse, NAME_None, true);
}

void ATelekinesisActor::Push(FVector direction)
{
	Mesh->AddImpulse(direction * PushForce, NAME_None, true);

	Mesh->SetEnableGravity(true);
	Mesh->SetAngularDamping(0.2f);
	Mesh->SetLinearDamping(0.2f);
}