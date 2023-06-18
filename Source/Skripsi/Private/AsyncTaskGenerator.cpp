// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncTaskGenerator.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
AAsyncTaskGenerator::AAsyncTaskGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAsyncTaskGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	// UE_LOG(LogTemp, Log, TEXT("%s"), *a);
	// UE_LOG(LogTemp, Log, TEXT("%i"), A1);
	// UE_LOG(LogTemp, Log, TEXT("result was %s"),(cell.wall[3] ? TEXT("true") : TEXT("false")));

	// StartAsyncTask();
	// StartAsyncTask_UsingAsyncTask();
}

// Called every frame
void AAsyncTaskGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAsyncTaskGenerator::StartAsyncTask()
{
	(new FAutoDeleteAsyncTask<FMyAsyncTask>(10))->StartBackgroundTask();
	UE_LOG(LogTemp, Log, TEXT("[MyLog] Stop - AsyncTask"));

}

void AAsyncTaskGenerator::StartAsyncTask_UsingAsyncTask()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []()
	{
		auto MyTask = new FAsyncTask<FMyAsyncTask>(10);
		MyTask->StartBackgroundTask();
		MyTask->EnsureCompletion();
		delete MyTask;
		UE_LOG(LogTemp, Log, TEXT("[MyLog] Stop - AsyncTaskManual"));
	});
}

void AAsyncTaskGenerator::GenerateMaze()
{
	FIntPoint startLoc(0, 0);
	cells.Empty();

	for (int i = 0; i < mazeSize.Y; ++i)
	{
		for (int j = 0; j < mazeSize.X; ++j)
		{
			cells.Add(FCell(FIntPoint(j, i)));
		}
	}

	TArray<FIntPoint> path;
	TArray<FIntPoint> deadEndLocs;
	bool haveNeighbour = false;

	cells[GetIndex(startLoc)].visited = true;
	path.Add(startLoc);
	
	FIntPoint currentLoc = startLoc;

	while (true)
	{
		while (GetUnvisitedNeighbours(currentLoc).Num() > 0)
		{
			TArray<FUnvisitedNeighbour> unvisitedNeighbours = GetUnvisitedNeighbours(currentLoc);
			int randomIndex = FMath::RandRange(0, unvisitedNeighbours.Num() - 1);
			FIntPoint nextLoc = unvisitedNeighbours[randomIndex].location;
			haveNeighbour = true;
		
			switch (unvisitedNeighbours[randomIndex].direction)
			{
				case EDirection::Up:
					cells[GetIndex(currentLoc)].wall[0] = false;
					cells[GetIndex(nextLoc)].wall[2] = false;
					break;
				case EDirection::Right:
					cells[GetIndex(currentLoc)].wall[1] = false;
					cells[GetIndex(nextLoc)].wall[3] = false;
					break;
				case EDirection::Down:
					cells[GetIndex(currentLoc)].wall[2] = false;
					cells[GetIndex(nextLoc)].wall[0] = false;
					break;
				case EDirection::Left:
					cells[GetIndex(currentLoc)].wall[3] = false;
					cells[GetIndex(nextLoc)].wall[1] = false;
					break;
			}

			currentLoc = nextLoc;
			cells[GetIndex(currentLoc)].visited = true;
			path.Add(currentLoc);
		}

		if (path.Num() > 0)
		{
			if (haveNeighbour)
			{
				deadEndLocs.Add(currentLoc);
				haveNeighbour = false;
			}
			currentLoc = path.Pop();
		}
		else
		{
			if (!perfect)
				BreakWall(deadEndLocs);
			return;
		}
	}
}

int AAsyncTaskGenerator::GetIndex(FIntPoint index2D)
{
	return index2D.Y * mazeSize.X + index2D.X;
}

TArray<FUnvisitedNeighbour> AAsyncTaskGenerator::GetUnvisitedNeighbours(FIntPoint loc)
{
	TArray<FUnvisitedNeighbour> unvisitedNeighbours;
	
	if (loc.X > 0)
	{
		FIntPoint neighbourLoc(loc.X - 1, loc.Y);
		if (!cells[GetIndex(neighbourLoc)].visited)
		{
			FUnvisitedNeighbour unvisitedNeighbour(neighbourLoc, EDirection::Left);
			unvisitedNeighbours.Add(unvisitedNeighbour);
		}
	}
	
	if (loc.X < mazeSize.X - 1)
	{
		FIntPoint neighbourLoc(loc.X + 1, loc.Y);
		if (!cells[GetIndex(neighbourLoc)].visited)
		{
			FUnvisitedNeighbour unvisitedNeighbour(neighbourLoc, EDirection::Right);
			unvisitedNeighbours.Add(unvisitedNeighbour);
		}
	}
	
	if (loc.Y > 0)
	{
		FIntPoint neighbourLoc(loc.X, loc.Y - 1);
		if (!cells[GetIndex(neighbourLoc)].visited)
		{
			FUnvisitedNeighbour unvisitedNeighbour(neighbourLoc, EDirection::Up);
			unvisitedNeighbours.Add(unvisitedNeighbour);
		}
	}
	
	if (loc.Y < mazeSize.Y - 1)
	{
		FIntPoint neighbourLoc(loc.X, loc.Y + 1);
		if (!cells[GetIndex(neighbourLoc)].visited)
		{
			FUnvisitedNeighbour unvisitedNeighbour(neighbourLoc, EDirection::Down);
			unvisitedNeighbours.Add(unvisitedNeighbour);
		}
	}

	return unvisitedNeighbours;
}

void AAsyncTaskGenerator::BreakWall(TArray<FIntPoint> deadEndLocs)
{
	for (auto cell : cells)
	{
		FIntPoint currentLoc = cell.location;

		if (currentLoc.Y > 0)
		{
			FIntPoint neighbourLoc(currentLoc.X, currentLoc.Y - 1);
			int wallIndex = 0;
			if (!deadEndLocs.Contains(currentLoc) && !deadEndLocs.Contains(neighbourLoc))
			{
				if (UKismetMathLibrary::RandomBoolWithWeight(breakWallChance))
				{
					cells[GetIndex(currentLoc)].wall[wallIndex] = false;
					cells[GetIndex(neighbourLoc)].wall[(wallIndex + 2) % 4] = false;
				}
			}
		}
		
		if (currentLoc.X < mazeSize.X - 1)
		{
			FIntPoint neighbourLoc(currentLoc.X + 1, currentLoc.Y);
			int wallIndex = 1;
			if (!deadEndLocs.Contains(currentLoc) && !deadEndLocs.Contains(neighbourLoc))
			{
				if (UKismetMathLibrary::RandomBoolWithWeight(breakWallChance))
				{
					cells[GetIndex(currentLoc)].wall[wallIndex] = false;
					cells[GetIndex(neighbourLoc)].wall[(wallIndex + 2) % 4] = false;
				}
			}
		}
		
		if (currentLoc.Y < mazeSize.Y - 1)
		{
			FIntPoint neighbourLoc(currentLoc.X, currentLoc.Y + 1);
			int wallIndex = 2;
			if (!deadEndLocs.Contains(currentLoc) && !deadEndLocs.Contains(neighbourLoc))
			{
				if (UKismetMathLibrary::RandomBoolWithWeight(breakWallChance))
				{
					cells[GetIndex(currentLoc)].wall[wallIndex] = false;
					cells[GetIndex(neighbourLoc)].wall[(wallIndex + 2) % 4] = false;
				}
			}
		}
		
		if (currentLoc.X > 0)
		{
			FIntPoint neighbourLoc(currentLoc.X - 1, currentLoc.Y);
			int wallIndex = 3;
			if (!deadEndLocs.Contains(currentLoc) && !deadEndLocs.Contains(neighbourLoc))
			{
				if (UKismetMathLibrary::RandomBoolWithWeight(breakWallChance))
				{
					cells[GetIndex(currentLoc)].wall[wallIndex] = false;
					cells[GetIndex(neighbourLoc)].wall[(wallIndex + 2) % 4] = false;
				}
			}
		}
	}
}

void AAsyncTaskGenerator::GenerateNPC(FSpawnNPCAttacker spawnNPCAttacker, FSpawnNPCDisturber spawnNPCDisturber)
{
	bool redo;
	do
	{
		redo = false;
		GenerateNPCLocs();
		GenerateNPCAttacker(spawnNPCAttacker);
		paths.Empty();
		paths = GetExitPaths();

		if (paths.Num() == 0)
			redo = true;
		else if (NPCAttackers.Num() > 0 && paths[0].intPoints.Num() == mazeSize.X + mazeSize.Y - 2)
			redo = true;
	}
	while (redo);

	do
	{
		GenerateNPCDisturber(spawnNPCDisturber);
	}
	while (!CanSpawnNPCDisturber());
}

void AAsyncTaskGenerator::GenerateNPCLocs()
{
	int minLeft = 0;
	int maxLeft = (mazeSize.X - 1) / 2;
	int minRight = maxLeft + 1;
	int maxRight = mazeSize.X - 1;
	int minTop = 0;
	int maxTop = (mazeSize.Y - 1) / 2;
	int minBot = maxTop + 1;
	int maxBot = mazeSize.Y - 1;

	NPCLocs.Empty();

	FIntPoint botLeft(FMath::RandRange(minLeft, maxLeft), FMath::RandRange(minBot, maxBot));
	NPCLocs.Add(botLeft);

	FIntPoint botRight(FMath::RandRange(minRight, maxRight), FMath::RandRange(minBot, maxBot));
	NPCLocs.Add(botRight);

	FIntPoint topRight(FMath::RandRange(minRight, maxRight), FMath::RandRange(minTop, maxTop));
	NPCLocs.Add(topRight);

	FIntPoint mid;
	do
	{
		mid = FIntPoint(FMath::RandRange(minLeft + 2, maxRight - 2), FMath::RandRange(minTop + 2, maxBot - 2));
	}
	while (NPCLocs.Contains(mid));
	NPCLocs.Add(mid);
}

void AAsyncTaskGenerator::GenerateNPCAttacker(FSpawnNPCAttacker spawnNPCAttacker)
{
	NPCAttackers.Empty();
	NPCCollisionDatas.Empty();

	if (spawnNPCAttacker.random)
	{
		for (int i = 0; i < spawnNPCAttacker.num; ++i)
		{
			FNPC NPC;
		
			if (UKismetMathLibrary::RandomBool())
				NPC.type = ENPCType::AttackerMelee;
			else
				NPC.type = ENPCType::AttackerRange;
		
			if (UKismetMathLibrary::RandomBool())
				NPC.movement = EMovementPriority::Horizontal;
			else
				NPC.movement = EMovementPriority::Vertical;

			int randomIndex = FMath::RandRange(0, NPCLocs.Num() - 1);
			NPC.spawnLoc = NPCLocs[randomIndex];
			NPCLocs.RemoveAt(randomIndex);

			NPCAttackers.Add(NPC);
		}
	}
	else
	{
		for (int i = 0; i < spawnNPCAttacker.numNPCMelee; ++i)
		{
			FNPC NPC;

			NPC.type = ENPCType::AttackerMelee;
		
			if (UKismetMathLibrary::RandomBool())
				NPC.movement = EMovementPriority::Horizontal;
			else
				NPC.movement = EMovementPriority::Vertical;

			int randomIndex = FMath::RandRange(0, NPCLocs.Num() - 1);
			NPC.spawnLoc = NPCLocs[randomIndex];
			NPCLocs.RemoveAt(randomIndex);
			
			NPCAttackers.Add(NPC);
		}

		for (int i = 0; i < spawnNPCAttacker.numNPCRange; ++i)
		{
			FNPC NPC;

			NPC.type = ENPCType::AttackerRange;
		
			if (UKismetMathLibrary::RandomBool())
				NPC.movement = EMovementPriority::Horizontal;
			else
				NPC.movement = EMovementPriority::Vertical;

			int randomIndex = FMath::RandRange(0, NPCLocs.Num() - 1);
			NPC.spawnLoc = NPCLocs[randomIndex];
			NPCLocs.RemoveAt(randomIndex);

			NPCAttackers.Add(NPC);
		}
	}
}

void AAsyncTaskGenerator::GenerateNPCDisturber(FSpawnNPCDisturber spawnNPCDisturber)
{
	NPCDisturbers.Empty();
	TArray<FIntPoint> NPCDisturberLocs = NPCLocs;

	if (spawnNPCDisturber.random)
	{
		for (int i = 0; i < spawnNPCDisturber.num; ++i)
		{
			FNPC NPC;
			
			if (UKismetMathLibrary::RandomBool())
				NPC.type = ENPCType::DisturberTrap;
			else
			{
				NPC.type = ENPCType::DisturberBomb;
				NPC.bombExplodeTime = bombExplodeTime;
			}

			int randomIndex = FMath::RandRange(0, NPCDisturberLocs.Num() - 1);
			NPC.spawnLoc = NPCDisturberLocs[randomIndex];
			NPCDisturberLocs.RemoveAt(randomIndex);

			FIntPoint obstacleLoc;
			bool same;
			do
			{
				same = false;
				if (UKismetMathLibrary::RandomBool())
					obstacleLoc = FIntPoint(FMath::RandRange(mazeSize.X / 3, mazeSize.X - 1),
											FMath::RandRange(0, mazeSize.Y - 1));
				else
					obstacleLoc = FIntPoint(FMath::RandRange(0, mazeSize.X - 1),
											FMath::RandRange(mazeSize.Y / 3, mazeSize.Y - 1));

				for (auto NPCDisturber : NPCDisturbers)
				{
					if (obstacleLoc == NPCDisturber.obstacleLoc)
					{
						same = true;
						break;
					}
				}

				if (!same)
				{
					if (obstacleLoc == NPC.spawnLoc || obstacleLoc == finishLoc)
						same = true;
				}
			}
			while (same);

			NPC.obstacleLoc = obstacleLoc;
			NPC.obstaclePath = GetShortestPath(NPC.spawnLoc, NPC.obstacleLoc);
			
			NPCDisturbers.Add(NPC);
		}
	}
	else
	{
		for (int i = 0; i < spawnNPCDisturber.numNPCTrap; ++i)
		{
			FNPC NPC;
			
			NPC.type = ENPCType::DisturberTrap;

			int randomIndex = FMath::RandRange(0, NPCDisturberLocs.Num() - 1);
			NPC.spawnLoc = NPCDisturberLocs[randomIndex];
			NPCDisturberLocs.RemoveAt(randomIndex);

			FIntPoint obstacleLoc;
			bool same;
			do
			{
				same = false;
				if (UKismetMathLibrary::RandomBool())
					obstacleLoc = FIntPoint(FMath::RandRange(mazeSize.X / 3, mazeSize.X - 1),
											FMath::RandRange(0, mazeSize.Y - 1));
				else
					obstacleLoc = FIntPoint(FMath::RandRange(0, mazeSize.X - 1),
											FMath::RandRange(mazeSize.Y / 3, mazeSize.Y - 1));

				for (auto NPCDisturber : NPCDisturbers)
				{
					if (obstacleLoc == NPCDisturber.obstacleLoc)
					{
						same = true;
						break;
					}
				}

				if (!same)
				{
					if (obstacleLoc == NPC.spawnLoc || obstacleLoc == finishLoc)
						same = true;
				}
			}
			while (same);

			NPC.obstacleLoc = obstacleLoc;
			NPC.obstaclePath = GetShortestPath(NPC.spawnLoc, NPC.obstacleLoc);
			
			NPCDisturbers.Add(NPC);
		}

		for (int i = 0; i < spawnNPCDisturber.numNPCBomb; ++i)
		{
			FNPC NPC;
			
			NPC.type = ENPCType::DisturberBomb;
			NPC.bombExplodeTime = bombExplodeTime;

			int randomIndex = FMath::RandRange(0, NPCDisturberLocs.Num() - 1);
			NPC.spawnLoc = NPCDisturberLocs[randomIndex];
			NPCDisturberLocs.RemoveAt(randomIndex);

			FIntPoint obstacleLoc;
			bool same;
			do
			{
				same = false;
				if (UKismetMathLibrary::RandomBool())
					obstacleLoc = FIntPoint(FMath::RandRange(mazeSize.X / 3, mazeSize.X - 1),
											FMath::RandRange(0, mazeSize.Y - 1));
				else
					obstacleLoc = FIntPoint(FMath::RandRange(0, mazeSize.X - 1),
											FMath::RandRange(mazeSize.Y / 3, mazeSize.Y - 1));

				for (auto NPCDisturber : NPCDisturbers)
				{
					if (obstacleLoc == NPCDisturber.obstacleLoc)
					{
						same = true;
						break;
					}
				}

				if (!same)
				{
					if (obstacleLoc == NPC.spawnLoc || obstacleLoc == finishLoc)
						same = true;
				}
			}
			while (same);

			NPC.obstacleLoc = obstacleLoc;
			NPC.obstaclePath = GetShortestPath(NPC.spawnLoc, NPC.obstacleLoc);
			
			NPCDisturbers.Add(NPC);
		}
	}
	
}

TArray<FArrayIntPoint> AAsyncTaskGenerator::GetExitPaths()
{
	TMap<FString, int> GScores;
	TMap<FString, int> FScores;
	TMap<FString, FString> parent;
	TArray<FString> stringPaths;
	TArray<FArrayIntPoint> intPointPaths;

	TArray<FIntPoint> start;
	start.Add(playerSpawnLoc);
	for (auto NPCAttacker : NPCAttackers)
	{
		start.Add(NPCAttacker.spawnLoc);
	}

	GScores.Add(ToString(start), 0);
	FScores.Add(ToString(start), ManhattanDistance(playerSpawnLoc, finishLoc));
	
	TArray<TArray<FIntPoint>> openList;
	TArray<TArray<FIntPoint>> closedList;

	openList.Add(start);
	
	while (openList.Num() > 0)
	{
		TArray<FIntPoint> current = openList[0];
		FIntPoint currentPlayerLoc = current[0];
		
		if (currentPlayerLoc == finishLoc)
		{
			FString locKey = ToString(current);
			stringPaths.Add(locKey);
			while (true)
			{
				locKey = parent[locKey];
				if (locKey != ToString(start))
				{
					stringPaths.Add(locKey);
				}
				else
				{
					Algo::Reverse(stringPaths);
					intPointPaths = ProcessPaths(stringPaths);
					return intPointPaths;
				}
			}
		}
		
		openList.RemoveAt(0);
		closedList.Add(current);

		if (!cells[GetIndex(currentPlayerLoc)].wall[0])
		{
			TArray<FIntPoint> neighbour;
			bool canAttack = false;

			FIntPoint neighbourPlayerLoc(currentPlayerLoc.X, currentPlayerLoc.Y - 1);
			neighbour.Add(neighbourPlayerLoc);

			int i = 1;
			for (auto NPCAttacker : NPCAttackers)
			{
				FIntPoint newNPCLoc = FindNPCAttackerLocation(NPCAttacker, neighbourPlayerLoc, current[i]);
				neighbour.Add(newNPCLoc);
				canAttack = CanNPCAttackPlayer(neighbourPlayerLoc, newNPCLoc, NPCAttacker);

				if (canAttack)
					break;
				
				i++;
			}

			if (!canAttack)
			{
				CheckNPCCollision(neighbour);
				if (!closedList.Contains(neighbour))
				{
					int GScore = GScores[ToString(current)] + 1;
					if (GScores.Contains(ToString(neighbour)) && GScore < GScores[ToString(neighbour)] ||
						!GScores.Contains(ToString(neighbour)))
					{
						GScores.Add(ToString(neighbour), GScore);
						FScores.Add(ToString(neighbour), GScore + ManhattanDistance(neighbourPlayerLoc, finishLoc));
						parent.Add(ToString(neighbour), ToString(current));

						if (!openList.Contains(neighbour))
						{
							AddToPriorityQueue(openList, neighbour, FScores);
						}
					}
				}
			}
		}
		
		if (!cells[GetIndex(currentPlayerLoc)].wall[1])
		{
			TArray<FIntPoint> neighbour;
			bool canAttack = false;

			FIntPoint neighbourPlayerLoc(currentPlayerLoc.X + 1, currentPlayerLoc.Y);
			neighbour.Add(neighbourPlayerLoc);
			
			int i = 1;
			for (auto NPCAttacker : NPCAttackers)
			{
				FIntPoint newNPCLoc = FindNPCAttackerLocation(NPCAttacker, neighbourPlayerLoc, current[i]);
				neighbour.Add(newNPCLoc);
				canAttack = CanNPCAttackPlayer(neighbourPlayerLoc, newNPCLoc, NPCAttacker);

				if (canAttack)
					break;
				
				i++;
			}

			if (!canAttack)
			{
				CheckNPCCollision(neighbour);
				if (!closedList.Contains(neighbour))
				{
					int GScore = GScores[ToString(current)] + 1;
					if (GScores.Contains(ToString(neighbour)) && GScore < GScores[ToString(neighbour)] ||
						!GScores.Contains(ToString(neighbour)))
					{
						GScores.Add(ToString(neighbour), GScore);
						FScores.Add(ToString(neighbour), GScore + ManhattanDistance(neighbourPlayerLoc, finishLoc));
						parent.Add(ToString(neighbour), ToString(current));

						if (!openList.Contains(neighbour))
						{
							AddToPriorityQueue(openList, neighbour, FScores);
						}
					}
				}
			}
		}
		
		if (!cells[GetIndex(currentPlayerLoc)].wall[2])
		{
			TArray<FIntPoint> neighbour;
			bool canAttack = false;

			FIntPoint neighbourPlayerLoc(currentPlayerLoc.X, currentPlayerLoc.Y + 1);
			neighbour.Add(neighbourPlayerLoc);

			int i = 1;
			for (auto NPCAttacker : NPCAttackers)
			{
				FIntPoint newNPCLoc = FindNPCAttackerLocation(NPCAttacker, neighbourPlayerLoc, current[i]);
				neighbour.Add(newNPCLoc);
				canAttack = CanNPCAttackPlayer(neighbourPlayerLoc, newNPCLoc, NPCAttacker);

				if (canAttack)
					break;
				
				i++;
			}

			if (!canAttack)
			{
				CheckNPCCollision(neighbour);
				if (!closedList.Contains(neighbour))
				{
					int GScore = GScores[ToString(current)] + 1;
					if (GScores.Contains(ToString(neighbour)) && GScore < GScores[ToString(neighbour)] ||
						!GScores.Contains(ToString(neighbour)))
					{
						GScores.Add(ToString(neighbour), GScore);
						FScores.Add(ToString(neighbour), GScore + ManhattanDistance(neighbourPlayerLoc, finishLoc));
						parent.Add(ToString(neighbour), ToString(current));

						if (!openList.Contains(neighbour))
						{
							AddToPriorityQueue(openList, neighbour, FScores);
						}
					}
				}
			}
		}
		
		if (!cells[GetIndex(currentPlayerLoc)].wall[3])
		{
			TArray<FIntPoint> neighbour;
			bool canAttack = false;

			FIntPoint neighbourPlayerLoc(currentPlayerLoc.X - 1, currentPlayerLoc.Y);
			neighbour.Add(neighbourPlayerLoc);

			int i = 1;
			for (auto NPCAttacker : NPCAttackers)
			{
				FIntPoint newNPCLoc = FindNPCAttackerLocation(NPCAttacker, neighbourPlayerLoc, current[i]);
				neighbour.Add(newNPCLoc);
				canAttack = CanNPCAttackPlayer(neighbourPlayerLoc, newNPCLoc, NPCAttacker);

				if (canAttack)
					break;
				
				i++;
			}

			if (!canAttack)
			{
				CheckNPCCollision(neighbour);
				if (!closedList.Contains(neighbour))
				{
					int GScore = GScores[ToString(current)] + 1;
					if (GScores.Contains(ToString(neighbour)) && GScore < GScores[ToString(neighbour)] ||
						!GScores.Contains(ToString(neighbour)))
					{
						GScores.Add(ToString(neighbour), GScore);
						FScores.Add(ToString(neighbour), GScore + ManhattanDistance(neighbourPlayerLoc, finishLoc));
						parent.Add(ToString(neighbour), ToString(current));

						if (!openList.Contains(neighbour))
						{
							AddToPriorityQueue(openList, neighbour, FScores);
						}
					}
				}
			}
		}
		
		TArray<FIntPoint> neighbour;
		bool canAttack = false;
		
		FIntPoint neighbourPlayerLoc = currentPlayerLoc;
		neighbour.Add(neighbourPlayerLoc);
		
		int i = 1;
		for (auto NPCAttacker : NPCAttackers)
		{
			FIntPoint newNPCLoc = FindNPCAttackerLocation(NPCAttacker, neighbourPlayerLoc, current[i]);
			neighbour.Add(newNPCLoc);
			canAttack = CanNPCAttackPlayer(neighbourPlayerLoc, newNPCLoc, NPCAttacker);

			if (canAttack)
				break;
				
			i++;
		}

		if (!canAttack)
		{
			CheckNPCCollision(neighbour);
			if (!closedList.Contains(neighbour))
			{
				int GScore = GScores[ToString(current)] + 1;
				if (GScores.Contains(ToString(neighbour)) && GScore < GScores[ToString(neighbour)] ||
					!GScores.Contains(ToString(neighbour)))
				{
					GScores.Add(ToString(neighbour), GScore);
					FScores.Add(ToString(neighbour), GScore + ManhattanDistance(neighbourPlayerLoc, finishLoc));
					parent.Add(ToString(neighbour), ToString(current));

					if (!openList.Contains(neighbour))
					{
						AddToPriorityQueue(openList, neighbour, FScores);
					}
				}
			}
		}
	}
	
	return intPointPaths;
}

FString AAsyncTaskGenerator::ToString(TArray<FIntPoint> locs)
{
	FString stringLocs;
	
	for (auto loc : locs)
	{
		FString stringX;
		if (loc.X < 10 && loc.X >= 0)
			stringX = "0" + FString::FromInt(loc.X);
		else
			stringX = FString::FromInt(loc.X);

		FString stringY;
		if (loc.Y < 10 && loc.Y >= 0)
			stringY = "0" + FString::FromInt(loc.Y);
		else
			stringY = FString::FromInt(loc.Y);

		stringLocs = stringLocs + stringX + stringY;
	}
	
	return stringLocs;
}

int AAsyncTaskGenerator::ManhattanDistance(FIntPoint loc1, FIntPoint loc2)
{
	return abs(loc1.X - loc2.X) + abs(loc1.Y - loc2.Y);
}

FIntPoint AAsyncTaskGenerator::FindNPCAttackerLocation(FNPC NPCAttacker, FIntPoint playerLoc, FIntPoint NPCLoc)
{
	if (NPCLoc.X < 0)
		return NPCLoc;

	switch (NPCAttacker.movement)
	{
		case EMovementPriority::Horizontal:
			if (!cells[GetIndex(NPCLoc)].wall[1] && playerLoc.X > NPCLoc.X)
			{
				return FIntPoint(NPCLoc.X + 1, NPCLoc.Y);
			}
			
			if (!cells[GetIndex(NPCLoc)].wall[3] && playerLoc.X < NPCLoc.X)
			{
				return FIntPoint(NPCLoc.X - 1, NPCLoc.Y);
			}

			if (!cells[GetIndex(NPCLoc)].wall[0] && playerLoc.Y < NPCLoc.Y)
			{
				return FIntPoint(NPCLoc.X, NPCLoc.Y - 1);
			}

			if (!cells[GetIndex(NPCLoc)].wall[2] && playerLoc.Y > NPCLoc.Y)
			{
				return FIntPoint(NPCLoc.X, NPCLoc.Y + 1);
			}
			break;
		case EMovementPriority::Vertical:
			if (!cells[GetIndex(NPCLoc)].wall[0] && playerLoc.Y < NPCLoc.Y)
			{
				return FIntPoint(NPCLoc.X, NPCLoc.Y - 1);
			}

			if (!cells[GetIndex(NPCLoc)].wall[2] && playerLoc.Y > NPCLoc.Y)
			{
				return FIntPoint(NPCLoc.X, NPCLoc.Y + 1);
			}
		
			if (!cells[GetIndex(NPCLoc)].wall[1] && playerLoc.X > NPCLoc.X)
			{
				return FIntPoint(NPCLoc.X + 1, NPCLoc.Y);
			}
			
			if (!cells[GetIndex(NPCLoc)].wall[3] && playerLoc.X < NPCLoc.X)
			{
				return FIntPoint(NPCLoc.X - 1, NPCLoc.Y);
			}
			break;
	}
	
	return NPCLoc;
}

bool AAsyncTaskGenerator::CanNPCAttackPlayer(FIntPoint playerLoc, FIntPoint NPCLoc, FNPC NPCAttacker)
{
	if (NPCLoc.X < 0)
		return false;

	switch (NPCAttacker.type)
	{
		case ENPCType::AttackerMelee:
			if (playerLoc == NPCLoc)
				return true;
			break;
		case ENPCType::AttackerRange:
			if (playerLoc == NPCLoc ||
				playerLoc.X == NPCLoc.X && playerLoc.Y == NPCLoc.Y - 1 && !cells[GetIndex(NPCLoc)].wall[0] ||
				playerLoc.X == NPCLoc.X && playerLoc.Y == NPCLoc.Y + 1 && !cells[GetIndex(NPCLoc)].wall[2] ||
				playerLoc.X == NPCLoc.X + 1 && playerLoc.Y == NPCLoc.Y && !cells[GetIndex(NPCLoc)].wall[1] ||
				playerLoc.X == NPCLoc.X - 1 && playerLoc.Y == NPCLoc.Y && !cells[GetIndex(NPCLoc)].wall[3])
					return true;
			break;
	}

	return false;
}

void AAsyncTaskGenerator::CheckNPCCollision(TArray<FIntPoint> &locs)
{
	TArray<TArray<int>> collidingNPCIndexes;
	
	for (int i = 1; i < locs.Num(); ++i)
	{
		if (locs[i].X < 0)
			continue;
		
		TArray<int> sameLocIndex;
		sameLocIndex.Add(i - 1);
		
		for (int j = i + 1; j < locs.Num(); ++j)
		{
			if (locs[i] == locs[j])
				sameLocIndex.Add(j - 1);
		}

		if (sameLocIndex.Num() > 1)
		{
			bool same = false;
			
			for (auto indexes : collidingNPCIndexes)
			{
				same = true;
				for (int j = 0; j < sameLocIndex.Num(); ++j)
				{
					if (!indexes.Contains(sameLocIndex[j]))
					{
						same = false;
						break;
					}
				}
				if (same)
					break;
			}
			
			if (!same)
				collidingNPCIndexes.Add(sameLocIndex);
		}
	}

	for (auto indexes : collidingNPCIndexes)
	{
		int highestPower = 0;
		TArray<int> samePowerIndexes;

		for (auto index : indexes)
		{
			if (GetNPCPower(NPCAttackers[index].type) > highestPower)
			{
				highestPower = GetNPCPower(NPCAttackers[index].type);
				for (auto samePowerIndex : samePowerIndexes)
				{
					locs[samePowerIndex + 1] = FIntPoint(-1, -1);
				}
				samePowerIndexes.Empty();
				samePowerIndexes.Add(index);
			}
			else if (GetNPCPower(NPCAttackers[index].type) < highestPower)
			{
				locs[index + 1] = FIntPoint(-1, -1);
			}
			else
			{
				samePowerIndexes.Add(index);
			}
		}
		
		if (samePowerIndexes.Num() > 1)
		{
			NPCCollisionDatas.Add(FNPCCollisionData(samePowerIndexes, samePowerIndexes[0]));
			
			for (int i = 1; i < samePowerIndexes.Num(); ++i)
			{
				locs[samePowerIndexes[i] + 1] = FIntPoint(-1, -1);
			}
		}
	}
}

int AAsyncTaskGenerator::GetNPCPower(ENPCType NPCType)
{
	switch (NPCType)
	{
	case ENPCType::AttackerMelee:
		return 3;
	case ENPCType::AttackerRange:
		return 4;
	case ENPCType::DisturberTrap:
		return 1;
	case ENPCType::DisturberBomb:
		return 2;
	}
	return 0;
}

void AAsyncTaskGenerator::AddToPriorityQueue(TArray<TArray<FIntPoint>> &queue, TArray<FIntPoint> value,
	TMap<FString, int> scores)
{
	if (queue.Num() == 0)
		queue.Add(value);
	else
	{
		TArray<TArray<FIntPoint>> tempQueue;
		int valueScore = scores[ToString(value)];
		bool added = false;
		int index = 0;
		
		for (int i = 0; i < queue.Num(); ++i)
		{
			if (scores[ToString(queue[i])] <= valueScore)
			{
				tempQueue.Add(queue[i]);
				added = true;
				index = i;
			}
			else
			{
				break;
			}
		}
		
		tempQueue.Add(value);
		
		if (added) index++;
		
		for (int i = index; i < queue.Num(); ++i)
		{
			tempQueue.Add(queue[i]);
		}

		queue = tempQueue;
	}
}

TArray<FArrayIntPoint> AAsyncTaskGenerator::ProcessPaths(TArray<FString> stringPaths)
{
	TArray<FArrayIntPoint> intPointPaths;
	
	int n = stringPaths[0].Len() / 4;
	for (int i = 0; i < n; ++i)
	{
		TArray<FIntPoint> path;
		for (auto stringPath : stringPaths)
		{
			int startIndex = i * 4;
			path.Add(FIntPoint(FCString::Atoi(*stringPath.Mid(startIndex, 2)),
									FCString::Atoi(*stringPath.Mid(startIndex + 2, 2))));
		}
		intPointPaths.Add(FArrayIntPoint(path));
	}

	return intPointPaths;
}

TArray<FIntPoint> AAsyncTaskGenerator::GetShortestPath(FIntPoint startLoc, FIntPoint endLoc)
{
	TArray<FIntPoint> path;
	TMap<FIntPoint, int> GScores;
	TMap<FIntPoint, int> FScores;
	TMap<FIntPoint, FIntPoint> parent;

	GScores.Add(startLoc, 0);
	FScores.Add(startLoc, ManhattanDistance(startLoc, endLoc));

	TArray<FIntPoint> openList;
	TArray<FIntPoint> closedList;

	openList.Add(startLoc);

	while (openList.Num() > 0)
	{
		FIntPoint current = openList[0];

		if (current == endLoc)
		{
			FIntPoint loc = endLoc;
			path.Add(loc);
			while (true)
			{
				loc = parent[loc];
				if (loc != startLoc)
				{
					path.Add(loc);
				}
				else
				{
					Algo::Reverse(path);
					return path;
				}
			}
		}

		openList.RemoveAt(0);
		closedList.Add(current);

		if (!cells[GetIndex(current)].wall[0])
		{
			FIntPoint neighbour(current.X, current.Y - 1);
			if (!closedList.Contains(neighbour))
			{
				int GScore = GScores[current] + 1;
				if (GScores.Contains(neighbour) && GScore < GScores[neighbour] ||
					!GScores.Contains(neighbour))
				{
					GScores.Add(neighbour, GScore);
					FScores.Add(neighbour, GScore + ManhattanDistance(neighbour, endLoc));
					parent.Add(neighbour, current);

					if (!openList.Contains(neighbour))
					{
						AddToPriorityQueueIntPoint(openList, neighbour, FScores);
					}
				}
			}
		}

		if (!cells[GetIndex(current)].wall[1])
		{
			FIntPoint neighbour(current.X + 1, current.Y);
			if (!closedList.Contains(neighbour))
			{
				int GScore = GScores[current] + 1;
				if (GScores.Contains(neighbour) && GScore < GScores[neighbour] ||
					!GScores.Contains(neighbour))
				{
					GScores.Add(neighbour, GScore);
					FScores.Add(neighbour, GScore + ManhattanDistance(neighbour, endLoc));
					parent.Add(neighbour, current);

					if (!openList.Contains(neighbour))
					{
						AddToPriorityQueueIntPoint(openList, neighbour, FScores);
					}
				}
			}
		}

		if (!cells[GetIndex(current)].wall[2])
		{
			FIntPoint neighbour(current.X, current.Y + 1);
			if (!closedList.Contains(neighbour))
			{
				int GScore = GScores[current] + 1;
				if (GScores.Contains(neighbour) && GScore < GScores[neighbour] ||
					!GScores.Contains(neighbour))
				{
					GScores.Add(neighbour, GScore);
					FScores.Add(neighbour, GScore + ManhattanDistance(neighbour, endLoc));
					parent.Add(neighbour, current);

					if (!openList.Contains(neighbour))
					{
						AddToPriorityQueueIntPoint(openList, neighbour, FScores);
					}
				}
			}
		}

		if (!cells[GetIndex(current)].wall[3])
		{
			FIntPoint neighbour(current.X - 1, current.Y);
			if (!closedList.Contains(neighbour))
			{
				int GScore = GScores[current] + 1;
				if (GScores.Contains(neighbour) && GScore < GScores[neighbour] ||
					!GScores.Contains(neighbour))
				{
					GScores.Add(neighbour, GScore);
					FScores.Add(neighbour, GScore + ManhattanDistance(neighbour, endLoc));
					parent.Add(neighbour, current);

					if (!openList.Contains(neighbour))
					{
						AddToPriorityQueueIntPoint(openList, neighbour, FScores);
					}
				}
			}
		}
	}

	return path;
}

void AAsyncTaskGenerator::AddToPriorityQueueIntPoint(TArray<FIntPoint>& queue, FIntPoint value,
	TMap<FIntPoint, int> scores)
{
	if (queue.Num() == 0)
		queue.Add(value);
	else
	{
		TArray<FIntPoint> tempQueue;
		int valueScore = scores[value];
		bool added = false;
		int index = 0;
		
		for (int i = 0; i < queue.Num(); ++i)
		{
			if (scores[queue[i]] <= valueScore)
			{
				tempQueue.Add(queue[i]);
				added = true;
				index = i;
			}
			else
			{
				break;
			}
		}
		
		tempQueue.Add(value);
		
		if (added) index++;
		
		for (int i = index; i < queue.Num(); ++i)
		{
			tempQueue.Add(queue[i]);
		}

		queue = tempQueue;
	}
}

bool AAsyncTaskGenerator::CanSpawnNPCDisturber()
{
	// check if npc disturber blocks exit path
	for (auto NPCDisturber : NPCDisturbers)
	{
		if (paths[0].intPoints[0] == NPCDisturber.spawnLoc)
			return false;

		int lastIndex = UKismetMathLibrary::Min(paths[0].intPoints.Num() - 1, NPCDisturber.obstaclePath.Num());
		for (int i = 0; i < lastIndex; ++i)
		{
			if (paths[0].intPoints[i + 1] == NPCDisturber.obstaclePath[i])
				return false;
		}
	}
	
	for (auto NPCDisturber : NPCDisturbers)
	{
		// check if bomb explosion hit/blocks player
		if (NPCDisturber.type == ENPCType::DisturberBomb)
		{
			if (DoesBombExplosionBlockThePath(NPCDisturber, paths[0].intPoints))
				return false;
		}
		else // check if trap blocks player
		{
			if (NPCDisturber.obstaclePath.Num() < paths[0].intPoints.Num())
			{
				for (int i = NPCDisturber.obstaclePath.Num(); i < paths[0].intPoints.Num(); ++i)
				{
					if (paths[0].intPoints[i] == NPCDisturber.obstacleLoc)
						return false;
				}
			}
		}
	}

	// check if bomb explosion blocks others npc path
	for (auto NPCDisturber : NPCDisturbers)
	{
		if (NPCDisturber.type == ENPCType::DisturberBomb)
		{
			for (auto otherNPCDisturber : NPCDisturbers)
			{
				if (DoesBombExplosionBlockThePath(NPCDisturber, otherNPCDisturber.obstaclePath))
					return false;
			}

			int i = 1;
			for (auto NPCAttacker : NPCAttackers)
			{
				if (DoesBombExplosionBlockThePath(NPCDisturber, paths[i].intPoints))
					return false;
				i++;
			}
		}
	}
	
	return true;
}

bool AAsyncTaskGenerator::DoesBombExplosionBlockThePath(FNPC NPCDisturber, TArray<FIntPoint> path)
{
	if (path.Num() >= NPCDisturber.obstaclePath.Num() + NPCDisturber.bombExplodeTime)
	{
		if (path[NPCDisturber.obstaclePath.Num() - 1 + NPCDisturber.bombExplodeTime] == NPCDisturber.obstacleLoc)
			return true;

		bool obstacleCellHasWall = false;
		for (int i = 0; i < 4; ++i)
		{
			if (cells[GetIndex(NPCDisturber.obstacleLoc)].wall[i])
			{
				obstacleCellHasWall = true;
				break;
			}
		}

		if (!obstacleCellHasWall)
			return false;

	
		for (int i = NPCDisturber.obstaclePath.Num() + NPCDisturber.bombExplodeTime; i < path.Num(); ++i)
		{
			if (path[i] == NPCDisturber.obstacleLoc)
				return true;
		}	
	}
	
	return false;
}

void AAsyncTaskGenerator::GenerateDynamicWallAndSwitch()
{
	bool success;
	
	FIntPoint currentLoc;
	FIntPoint nextLoc;
	FIntPoint switchLoc;
	EDirection dir;
	bool open;
	do
	{
		 int randomIndexWall = FMath::RandRange(paths[0].intPoints.Num() / 3, paths[0].intPoints.Num() - 2);
		 currentLoc = paths[0].intPoints[randomIndexWall];
		
		 int n = 1;
		 do
		 {
		 	nextLoc = paths[0].intPoints[randomIndexWall + n];
		 	n++;
		 }
		 while (nextLoc == currentLoc);

		success = !DoesDynamicWallBlockNPCDisturberPath(currentLoc, nextLoc);
		if (!success) continue;

		if (currentLoc.Y > nextLoc.Y)
			dir = EDirection::Up;
		else if (currentLoc.Y < nextLoc.Y)
			dir = EDirection::Down;
		else if (currentLoc.X < nextLoc.X)
			dir = EDirection::Right;
		else
			dir = EDirection::Left;

		int randomIndexSwitch = UKismetMathLibrary::Max(FMath::RandRange(2, randomIndexWall - 2), 1);
		switchLoc = paths[0].intPoints[randomIndexSwitch];

		 int sum = 0;
		 for (int i = 0; i <= randomIndexWall; ++i)
		 {
		 	if (paths[0].intPoints[i] == switchLoc) sum++;
		 }
		
		 open = false;
		 if (sum % 2 == 0) open = true;

		success = !DoesDynamicWallBlockNPCAttackerPath(currentLoc, nextLoc, switchLoc, open);
	}
	while (!success);

	dynamicWall = FDynamicWall(open, currentLoc, nextLoc, dir);
	switch_ = FSwitch(open, switchLoc);
}

bool AAsyncTaskGenerator::DoesDynamicWallBlockNPCDisturberPath(FIntPoint currentLoc, FIntPoint nextLoc)
{
	for (auto NPCDisturber : NPCDisturbers)
	{
		if (NPCDisturber.spawnLoc == currentLoc && NPCDisturber.obstaclePath[0] == nextLoc ||
			NPCDisturber.spawnLoc == nextLoc && NPCDisturber.obstaclePath[0] == currentLoc)
			return true;

		for (int i = 0; i < NPCDisturber.obstaclePath.Num() - 1; ++i)
		{
			if (NPCDisturber.obstaclePath[i] == currentLoc && NPCDisturber.obstaclePath[i + 1] == nextLoc ||
				NPCDisturber.obstaclePath[i] == nextLoc && NPCDisturber.obstaclePath[i + 1] == currentLoc)
				return true;
		}
	}

	return false;
}

bool AAsyncTaskGenerator::DoesDynamicWallBlockNPCAttackerPath(FIntPoint currentLoc, FIntPoint nextLoc,
	FIntPoint switchLoc, bool dynamicWallOpen)
{
	int n = 1;
	
	for (auto NPCAttacker : NPCAttackers)
	{
		if (NPCAttacker.spawnLoc == currentLoc && paths[n].intPoints[0] == nextLoc ||
			NPCAttacker.spawnLoc == nextLoc && paths[n].intPoints[0] == currentLoc)
		{
			if (!dynamicWallOpen)
				return true;
		}

		for (int i = 0; i < paths[n].intPoints.Num() - 1; ++i)
		{
			if (paths[n].intPoints[i] == currentLoc && paths[n].intPoints[i + 1] == nextLoc ||
				paths[n].intPoints[i] == nextLoc && paths[n].intPoints[i + 1] == currentLoc)
			{
				int sum = 0;
				for (int j = 0; j <= i; ++j)
				{
					if (paths[0].intPoints[j] == switchLoc) sum++;
				}
				
				if (sum % 2 == 0)
				{
					if (!dynamicWallOpen)
						return true;
				}
				else
				{
					if (dynamicWallOpen)
						return true;
				}
			}
		}
		
		n++;
	}

	return false;
}

void AAsyncTaskGenerator::Start(FIntPoint mSize, bool pf, float bwChance, int bombTime,
	FSpawnNPCAttacker spawnNPCAttacker, FSpawnNPCDisturber spawnNPCDisturber,  bool generateDynamicWallAndSwitch)
{
	mazeSize = mSize;
	perfect = pf;
	breakWallChance = bwChance;
	bombExplodeTime = bombTime;

	playerSpawnLoc = FIntPoint(0, 0);
	finishLoc = FIntPoint(mazeSize.X - 1, mazeSize.Y - 1);

	dynamicWall = FDynamicWall();
	switch_ = FSwitch();

	GenerateMaze();
	GenerateNPC(spawnNPCAttacker, spawnNPCDisturber);
	if (generateDynamicWallAndSwitch)
		GenerateDynamicWallAndSwitch();
}

void FMyAsyncTask::DoWork()
{
	while (LoopCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyLog] Run - NonAbandonableTask: %d"), LoopCount--);
		FPlatformProcess::Sleep(0.5f);
	}
	UE_LOG(LogTemp, Log, TEXT("[MyLog] Stop - NonAbandonableTask"));
}
