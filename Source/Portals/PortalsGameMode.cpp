// Copyright Epic Games, Inc. All Rights Reserved.

#include "PortalsGameMode.h"
#include "PortalsHUD.h"
#include "PortalsCharacter.h"
#include "UObject/ConstructorHelpers.h"

APortalsGameMode::APortalsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = APortalsHUD::StaticClass();
}
