// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSocketGameMode.h"
#include "CustomSocketCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACustomSocketGameMode::ACustomSocketGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
