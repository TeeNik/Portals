#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class USceneCaptureComponent2D;
class IPortable;
class UBoxComponent;

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

	UPROPERTY(EditAnywhere)
	APortal* Target;
	UPROPERTY(EditAnywhere)
	float ClipPlaneOffset = -1.5f;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal")
	bool IsPointInFrontOfPortal(FVector Point) const;

	bool IsPointCrossingPortal(IPortable* Portable);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	void Update(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneCaptureComponent2D* SceneCapture;

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
	TArray<IPortable*> PortableTargets;
	bool TickInProgress = false;

	void GeneratePortalTexture();
	FMatrix GetCameraProjectionMatrix();
	void UpdateCapture();
};
