// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Portable.h"
#include "PortableActor.generated.h"

UCLASS()
class PORTALS_API APortableActor : public AActor, public IPortable
{
	GENERATED_BODY()
	
public:	
	APortableActor();

	UFUNCTION(BlueprintCallable)
	virtual AActor* GetCopy() override;

};