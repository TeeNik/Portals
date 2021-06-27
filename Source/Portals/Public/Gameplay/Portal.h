#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class USceneCaptureComponent2D;

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


	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	void Update(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneCaptureComponent2D* SceneCapture;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* PortalRootComponent;

	UPROPERTY(Transient)
	UTextureRenderTarget2D* PortalTexture = nullptr;

private:
	void GeneratePortalTexture();
	FMatrix GetCameraProjectionMatrix();
	void UpdateCapture(USceneCaptureComponent2D* capture, UTextureRenderTarget2D* texture, AActor* target);

	FVector LastPosition;
	bool LastInFront;
};
