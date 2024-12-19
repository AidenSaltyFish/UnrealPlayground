// Copyright Epic Games, Inc. All Rights Reserved.

#include "YoshiGameMode.h"
#include "YoshiCharacter.h"
#include "UObject/ConstructorHelpers.h"

AYoshiGameMode::AYoshiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
