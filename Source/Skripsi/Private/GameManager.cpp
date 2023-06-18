// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"

#include "Kismet/KismetStringLibrary.h"

// Sets default values
AGameManager::AGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AGameManager::IsUpper(FString string)
{
	char* result = TCHAR_TO_ANSI(*string);
	return bool(std::isupper(result[0]));
}

bool AGameManager::IsSyntaxValid(FString playerSyntax, FString initialSyntax, TMap<FString, FArrayString> syntaxList, int varIndex)
{
	TCHAR ch = initialSyntax[varIndex];
	for (auto syntax : syntaxList[FString(&ch)].strings)
	{
		FString syntaxChild = initialSyntax.Left(varIndex) + syntax + initialSyntax.Right(initialSyntax.Len() - 1 - varIndex);

		if (syntaxChild.Len() <= playerSyntax.Len())
		{
			bool isSame = true;
					
			for (int i = varIndex; i < syntaxChild.Len(); ++i)
			{
				if (IsUpper(syntaxChild.Mid(i, 1)))
				{
					// UE_LOG(LogTemp, Log, TEXT("%s"), *syntaxChild);
					isSame = false;
							
					if (IsSyntaxValid(playerSyntax, syntaxChild, syntaxList, i))
						return true;
							
					break;
				}
				
				if (!UKismetStringLibrary::EqualEqual_StrStr(syntaxChild.Mid(i, 1), playerSyntax.Mid(i, 1)))
				{
					isSame = false;
					break;
				}
			}

			if (isSame && syntaxChild.Len() == playerSyntax.Len())
				return true;
		}
	}
	
	return false;
}

