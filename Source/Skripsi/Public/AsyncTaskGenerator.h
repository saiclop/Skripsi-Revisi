// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AsyncTaskGenerator.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Up		UMETA(DisplayName="Up"),
	Right	UMETA(DisplayName="Right"),
	Down	UMETA(DisplayName="Down"),
	Left	UMETA(DisplayName="Left")
};

UENUM(BlueprintType)
enum class ENPCType : uint8
{
	AttackerMelee	UMETA(DisplayName="Attacker Melee"),
	AttackerRange	UMETA(DisplayName="Attacker Range"),
	DisturberTrap	UMETA(DisplayName="Disturber Trap"),
	DisturberBomb	UMETA(DisplayName="Disturber Bomb")
};

UENUM(BlueprintType)
enum class EMovementPriority : uint8
{
	Horizontal		UMETA(DisplayName="Horizontal"),
	Vertical		UMETA(DisplayName="Vertical")
};

USTRUCT(Blueprintable)
struct FCell
{
	GENERATED_BODY()
	FIntPoint location;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<bool> wall;
	
	bool visited;

	FCell(FIntPoint loc = FIntPoint(0,0))
	{
		location = loc;
		visited = false;
		wall.Init(true, 4);
	}
};

USTRUCT(Blueprintable)
struct FNPC
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENPCType type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint spawnLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMovementPriority movement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint obstacleLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> obstaclePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int bombExplodeTime;

	FNPC()
	{
		type = ENPCType::AttackerMelee;
		spawnLoc = FIntPoint(0, 0);
		movement = EMovementPriority::Horizontal;
		obstacleLoc = FIntPoint(0, 0);
		bombExplodeTime = 0;
	}
};

USTRUCT(Blueprintable)
struct FNPCCollisionData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int> NPCIndexes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int winnerNPCIndex;

	FNPCCollisionData()
	{
		winnerNPCIndex = 0;
	}

	FNPCCollisionData(TArray<int> NPCIndexes, int winnerNPCIndex)
	{
		this->NPCIndexes = NPCIndexes;
		this->winnerNPCIndex = winnerNPCIndex;
	}
};

USTRUCT(Blueprintable)
struct FSwitch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool open;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint location;

	FSwitch(bool open = false, FIntPoint loc = FIntPoint(0, 0))
	{
		this->open = open;
		location = loc;
	}
};

USTRUCT(Blueprintable)
struct FDynamicWall
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool open;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint currentLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint nextLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDirection direction;

	FDynamicWall(bool open = false, FIntPoint currentLoc = FIntPoint(0, 0), FIntPoint nextLoc = FIntPoint(0, 0), EDirection direction = EDirection::Up)
	{
		this->open = open;
		this->currentLoc = currentLoc;
		this->nextLoc = nextLoc;
		this->direction = direction;
	}
};

USTRUCT(Blueprintable)
struct FArrayIntPoint
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> intPoints;

	FArrayIntPoint(){}

	FArrayIntPoint(TArray<FIntPoint> intPoints)
	{
		this->intPoints = intPoints;
	}
};

USTRUCT(Blueprintable)
struct FSpawnNPCAttacker
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool random;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int num;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numNPCMelee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numNPCRange;

	FSpawnNPCAttacker()
	{
		random = true;
		num = 0;
		numNPCMelee = 0;
		numNPCRange = 0;
	}
	
};

USTRUCT(Blueprintable)
struct FSpawnNPCDisturber
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool random;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int num;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numNPCTrap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numNPCBomb;

	FSpawnNPCDisturber()
	{
		random = true;
		num = 0;
		numNPCBomb = 0;
		numNPCTrap = 0;
	}
	
};

USTRUCT()
struct FUnvisitedNeighbour
{
	GENERATED_BODY()
	FIntPoint location;
	EDirection direction;

	FUnvisitedNeighbour(){}

	FUnvisitedNeighbour(FIntPoint loc, EDirection dir)
	{
		location = loc;
		direction = dir;
	}
};

UCLASS()
class SKRIPSI_API AAsyncTaskGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAsyncTaskGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void StartAsyncTask();
	void StartAsyncTask_UsingAsyncTask();

	void GenerateMaze();
	int GetIndex(FIntPoint index2D);
	TArray<FUnvisitedNeighbour> GetUnvisitedNeighbours(FIntPoint loc);
	void BreakWall(TArray<FIntPoint> deadEndLocs);

	void GenerateNPC(FSpawnNPCAttacker spawnNPCAttacker, FSpawnNPCDisturber spawnNPCDisturber);
	void GenerateNPCLocs();
	void GenerateNPCAttacker(FSpawnNPCAttacker spawnNPCAttacker);
	void GenerateNPCDisturber(FSpawnNPCDisturber spawnNPCDisturber);

	TArray<FArrayIntPoint> GetExitPaths();
	FString ToString(TArray<FIntPoint> locs);
	int ManhattanDistance(FIntPoint loc1, FIntPoint loc2);
	FIntPoint FindNPCAttackerLocation(FNPC NPCAttacker, FIntPoint playerLoc, FIntPoint NPCLoc);
	bool CanNPCAttackPlayer(FIntPoint playerLoc, FIntPoint NPCLoc, FNPC NPCAttacker);
	void CheckNPCCollision(TArray<FIntPoint> &NPCLocs);
	int GetNPCPower(ENPCType NPCType);
	void AddToPriorityQueue(TArray<TArray<FIntPoint>> &queue, TArray<FIntPoint> value, TMap<FString, int> scores);
	TArray<FArrayIntPoint> ProcessPaths(TArray<FString> stringPaths);
	TArray<FIntPoint> GetShortestPath(FIntPoint startLoc, FIntPoint endLoc);
	void AddToPriorityQueueIntPoint(TArray<FIntPoint> &queue, FIntPoint value, TMap<FIntPoint, int> scores);
	bool CanSpawnNPCDisturber();
	bool DoesBombExplosionBlockThePath(FNPC NPCDisturber, TArray<FIntPoint> path);

	void GenerateDynamicWallAndSwitch();
	bool DoesDynamicWallBlockNPCDisturberPath(FIntPoint currentLoc, FIntPoint nextLoc);
	bool DoesDynamicWallBlockNPCAttackerPath(FIntPoint currentLoc, FIntPoint nextLoc, FIntPoint switchLoc, bool dynamicWallOpen);

	UFUNCTION(BlueprintCallable)
	void Start(FIntPoint mazeSize, bool perfect, float breakWallChance, int bombExplodeTime, FSpawnNPCAttacker spawnNPCAttacker, FSpawnNPCDisturber spawnNPCDisturber, bool generateDynamicWallAndSwitch);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint mazeSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool perfect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float breakWallChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCell> cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FArrayIntPoint> paths;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNPC> NPCAttackers;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNPC> NPCDisturbers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int bombExplodeTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNPCCollisionData> NPCCollisionDatas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSwitch switch_;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDynamicWall dynamicWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint playerSpawnLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint finishLoc;
	
	TArray<FIntPoint> NPCLocs;
};

class FMyAsyncTask : public FNonAbandonableTask
{
public:
	FMyAsyncTask(int32 InLoopCount) : LoopCount(InLoopCount) {}
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FMyAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }
	void DoWork();

private:
	int32 LoopCount;
	
};


