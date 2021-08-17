#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class USceneCaptureComponent2D;
class IPortable;
class UBoxComponent;
class UPortableComponent;

UCLASS()
class PORTALS_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Portal")
	bool IsActive;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void ClearRTT();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void SetRTT(UTexture* RenderTexture);
	UFUNCTION(BlueprintNativeEvent, Category = "Portal")
	void ForceTick();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APortal* Target;
	UPROPERTY(EditAnywhere)
	float ClipPlaneOffset = -1.5f;
	UPROPERTY(EditAnywhere)
	int32 RecursionAmount = 2;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal")
	bool IsPointInFrontOfPortal(FVector Point) const;

	bool IsPointCrossingPortal(UPortableComponent* Portable);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneCaptureComponent2D* SceneCapture;

	UFUNCTION(BlueprintImplementableEvent)
	void SetMaterialScale(float scale);
	UFUNCTION(BlueprintImplementableEvent)
	void OnTeleportUsed(bool isTarget);

	void UpdateCapture();
	void AddPortableTarget(UPortableComponent* portable);
	FVector ConvertLocationToActorSpace(const FVector& actorLocation, const FTransform& source, const FTransform& target);
	FQuat ConvertRotationToActorSpace(const FRotator& actorRotation, const FTransform& source, const FTransform& target);
	FVector ConvertDirectionToTarget(FVector direction);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* PortalRootComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PortalView;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBoxComponent* InteractionBox;

	UPROPERTY(Transient)
	UTextureRenderTarget2D* PortalTexture = nullptr;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	TArray<UPortableComponent*> PortableTargets;
	bool TickInProgress = false;

	void GeneratePortalTexture();

};
