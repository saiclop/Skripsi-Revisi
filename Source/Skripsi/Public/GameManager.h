// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/Char.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

USTRUCT(Blueprintable)
struct FArrayString
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> strings;

	FArrayString(){}

	FArrayString(TArray<FString> strings)
	{
		this->strings = strings;
	}
};

UCLASS()
class SKRIPSI_API AGameManager : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AGameManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	bool IsUpper(FString string);

	UFUNCTION(BlueprintCallable)
	bool IsSyntaxValid(FString playerSyntax, FString initialSyntax, TMap<FString, FArrayString> syntaxList, int varIndex);
};
