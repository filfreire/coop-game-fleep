// Fill out your copyright notice in the Description page of Project Settings.

#include "MyTrainingEnvironment.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "SCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UMyTrainingEnvironment::UMyTrainingEnvironment()
{
	// Default target location (you can set this in Blueprint or editor)
	TargetLocation = FVector(0.0f, 0.0f, 0.0f);
	TargetActor = nullptr;
	
	// Reward configuration defaults
	GoalReachDistance = 200.0f;       // Distance to target to consider "reached"
	MaxEpisodeTime = 30.0f;           // Maximum episode duration in seconds
	MovementRewardScale = 0.1f;       // Scale for distance-based rewards
	GoalRewardAmount = 100.0f;        // Reward for reaching the goal
	TimePenaltyPerSecond = -0.01f;    // Small time penalty to encourage efficiency
	
	// Reset configuration
	SpawnRadius = 500.0f;             // Radius around spawn points for randomization
	MinDistanceBetweenAgents = 300.0f; // Minimum distance between spawned agents
	
	// Default spawn points (you should set these in Blueprint)
	SpawnPoints.Add(FVector(0.0f, 0.0f, 100.0f));
}

void UMyTrainingEnvironment::GatherAgentReward_Implementation(float& OutReward, const int32 AgentId)
{
	// Get reference to the character agent
	ASCharacter* Agent = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (Agent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyTrainingEnvironment: Failed to cast agent to ASCharacter"));
		OutReward = 0.0f;
		return;
	}

	const FVector AgentLocation = Agent->GetActorLocation();
	
	// Determine target location (use actor if set, otherwise use fixed location)
	FVector CurrentTargetLocation = TargetLocation;
	if (TargetActor.IsValid())
	{
		CurrentTargetLocation = TargetActor->GetActorLocation();
	}
	
	const float DistanceToTarget = FVector::Dist(AgentLocation, CurrentTargetLocation);

	// Reward for reaching the target
	float GoalReward = 0.0f;
	if (DistanceToTarget <= GoalReachDistance)
	{
		GoalReward = GoalRewardAmount; // Large reward for reaching the goal
	}

	// Reward for moving closer to target (based on inverse distance)
	const float MaxDistance = 10000.0f; // Maximum expected distance for normalization
	const float ProximityReward = ULearningAgentsRewards::MakeRewardFromLocationDifference(
		AgentLocation, CurrentTargetLocation, MaxDistance, MovementRewardScale);

	// Small penalty for time to encourage efficiency
	const float TimePenalty = TimePenaltyPerSecond;

	// Penalty for being idle (low velocity)
	const FVector Velocity = Agent->GetVelocity();
	const float Speed = Velocity.Size();
	const float IdlePenalty = Speed < 50.0f ? -0.1f : 0.0f;

	// Sum all rewards
	OutReward = GoalReward + ProximityReward + TimePenalty + IdlePenalty;
}

void UMyTrainingEnvironment::GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId)
{
	// Get reference to the character agent
	ASCharacter* Agent = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (Agent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyTrainingEnvironment: Failed to cast agent to ASCharacter"));
		OutCompletion = ELearningAgentsCompletion::Truncation;
		return;
	}

	const FVector AgentLocation = Agent->GetActorLocation();
	
	// Determine target location (use actor if set, otherwise use fixed location)
	FVector CurrentTargetLocation = TargetLocation;
	if (TargetActor.IsValid())
	{
		CurrentTargetLocation = TargetActor->GetActorLocation();
	}
	
	const float DistanceToTarget = FVector::Dist(AgentLocation, CurrentTargetLocation);

	// Check if agent reached the target
	if (DistanceToTarget <= GoalReachDistance)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Check for timeout
	const float* EpisodeStartTime = EpisodeStartTimes.Find(AgentId);
	if (EpisodeStartTime != nullptr)
	{
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		const float ElapsedTime = CurrentTime - *EpisodeStartTime;
		
		if (ElapsedTime >= MaxEpisodeTime)
		{
			OutCompletion = ELearningAgentsCompletion::Truncation;
			return;
		}
	}

	// Check if agent fell off the map or died
	if (Agent->IsDead() || AgentLocation.Z < -1000.0f)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Continue episode
	OutCompletion = ELearningAgentsCompletion::Running;
}

void UMyTrainingEnvironment::ResetAgentEpisode_Implementation(const int32 AgentId)
{
	// Get reference to the character agent
	ASCharacter* Agent = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (Agent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyTrainingEnvironment: Failed to cast agent to ASCharacter"));
		return;
	}

	// Get all agents to avoid overlapping spawns
	TArray<UObject*> AllObjects;
	TArray<int32> AllIds;
	Manager->GetAllAgents(AllObjects, AllIds, ASCharacter::StaticClass());

	// Cast to array of actors
	TArray<AActor*> AllAgents;
	AllAgents.Reserve(AllObjects.Num());
	for (UObject* Other : AllObjects)
	{
		if (AActor* OtherActor = Cast<AActor>(Other))
		{
			AllAgents.Add(OtherActor);
		}
	}

	// Find a valid spawn location
	const FVector SpawnLocation = GetValidSpawnLocation(AllAgents);
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	// Reset agent's health and state
	if (Agent->IsDead())
	{
		// If the character has a revive/reset function, call it here
		Agent->ResetCharacterPosition();
	}

	// Teleport agent to spawn location
	Agent->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
	
	// Reset velocity
	Agent->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	// Record episode start time
	EpisodeStartTimes.Add(AgentId, GetWorld()->GetTimeSeconds());
}

FVector UMyTrainingEnvironment::GetValidSpawnLocation(const TArray<AActor*>& AllAgents) const
{
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyTrainingEnvironment: No spawn points defined, using origin"));
		return FVector::ZeroVector;
	}

	const int32 MaxAttempts = 50;
	
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Pick a random spawn point
		const int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		const FVector BaseSpawnPoint = SpawnPoints[SpawnIndex];
		
		// Add random offset within spawn radius
		const FVector RandomOffset = FVector(
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			0.0f
		);
		
		const FVector CandidateLocation = BaseSpawnPoint + RandomOffset;
		
		// Check if location is clear of other agents
		bool bLocationValid = true;
		
		for (const AActor* OtherAgent : AllAgents)
		{
			if (OtherAgent != nullptr)
			{
				const float Distance = FVector::Dist(CandidateLocation, OtherAgent->GetActorLocation());
				if (Distance < MinDistanceBetweenAgents)
				{
					bLocationValid = false;
					break;
				}
			}
		}
		
		if (bLocationValid)
		{
			return CandidateLocation;
		}
	}
	
	// If we couldn't find a valid location, just use the first spawn point
	UE_LOG(LogTemp, Warning, TEXT("MyTrainingEnvironment: Could not find valid spawn location, using default"));
	return SpawnPoints[0];
}

