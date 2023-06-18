// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkripsiGameMode.h"
#include "SkripsiCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkripsiGameMode::ASkripsiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
