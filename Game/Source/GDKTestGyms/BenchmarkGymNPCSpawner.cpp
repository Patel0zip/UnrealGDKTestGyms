// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "BenchmarkGymNPCSpawner.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "DeterministicBlackboardValues.h"

DEFINE_LOG_CATEGORY(LogBenchmarkGymNPCSpawner);

ABenchmarkGymNPCSpawner::ABenchmarkGymNPCSpawner()
	: NumSpawned(0)
	, NumToSpawn(0)
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<APawn> NPCBPClass(TEXT("/Game/Characters/SimulatedPlayers/BenchmarkNPC_BP"));
	if (NPCBPClass.Class != NULL)
	{
		NPCPawnClass = NPCBPClass.Class;
	}
	NumSpawned = NumToSpawn = 0;
}

void ABenchmarkGymNPCSpawner::SpawnNPC(const FVector& SpawnLocation, const FBlackboardValues& BlackboardValues)
{
	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogBenchmarkGymNPCSpawner, Error, TEXT("Error spawning NPC, World is null"));
		return;
	}

	const float RandomSpawnOffset = 600.0f;
	FVector RandomOffset = FMath::VRand()*RandomSpawnOffset;
	if (RandomOffset.Z < 0.0f)
	{
		RandomOffset.Z = -RandomOffset.Z;
	}

	FVector FixedSpawnLocation = SpawnLocation + RandomOffset;
	UE_LOG(LogBenchmarkGymNPCSpawner, Warning, TEXT("Spawning NPC at %s"), *SpawnLocation.ToString());
	FActorSpawnParameters SpawnInfo{};
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APawn* Pawn = World->SpawnActor<APawn>(NPCPawnClass->GetDefaultObject()->GetClass(), FixedSpawnLocation, FRotator::ZeroRotator, SpawnInfo);
	checkf(Pawn, TEXT("Pawn failed to spawn at %s"), *FixedSpawnLocation.ToString());

	UDeterministicBlackboardValues* Comp = Cast<UDeterministicBlackboardValues>(Pawn->FindComponentByClass(UDeterministicBlackboardValues::StaticClass()));
	checkf(Comp, TEXT("Pawn must have a UDeterministicBlackboardValues component."));
	Comp->ClientSetBlackboardAILocations(BlackboardValues);
}

void ABenchmarkGymNPCSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (NumSpawned < NumToSpawn)
	{
		int32 NPCIndex = NumSpawned++;
		int32 Cluster = NPCIndex % NumClusters;
		int32 SpawnPointIndex = Cluster * PlayerDensity;

		SpawnNPC(SpawnLocations.GetSpawnPoint(NPCIndex), SpawnLocations.GetRunBetweenPoints(NPCIndex));
	}
}