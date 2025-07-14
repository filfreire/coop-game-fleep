// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterTrainingEnvironment.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsCompletions.h"
#include "STargetActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SCharacter.h"

USCharacterTrainingEnvironment::USCharacterTrainingEnvironment()
{
	TargetActor = nullptr;
}

void USCharacterTrainingEnvironment::GatherAgentReward_Implementation(float& OutReward, const int32 AgentId)
{
	OutReward = 0.0f;

	// Get the character agent
	ASCharacter* Character = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (!Character || !TargetActor)
	{
		return;
	}

	FVector CharacterLocation = Character->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();
	float CurrentDistance = FVector::Dist(CharacterLocation, TargetLocation);

	// Check if agent reached the target
	if (TargetActor->IsLocationWithinReach(CharacterLocation))
	{
		OutReward += ReachTargetReward;
		UE_LOG(LogTemp, Log, TEXT("Agent %d reached target! Reward: %f"), AgentId, ReachTargetReward);
	}
	else
	{
		// Distance-based reward (closer = better)
		float MaxDistance = FVector::Dist(ResetCenter - ResetBounds, ResetCenter + ResetBounds);
		float NormalizedDistance = FMath::Clamp(CurrentDistance / MaxDistance, 0.0f, 1.0f);
		OutReward += (1.0f - NormalizedDistance) * DistanceRewardScale;

		// Movement towards target reward
		if (PreviousDistances.Contains(AgentId))
		{
			float PreviousDistance = PreviousDistances[AgentId];
			if (CurrentDistance < PreviousDistance)
			{
				OutReward += MovementTowardsTargetReward;
			}
		}
	}

	// Time step penalty to encourage efficiency
	OutReward += TimeStepPenalty;

	// Update previous distance for next step
	PreviousDistances.Add(AgentId, CurrentDistance);

	// Increment episode step counter
	if (EpisodeSteps.Contains(AgentId))
	{
		EpisodeSteps[AgentId]++;
	}
	else
	{
		EpisodeSteps.Add(AgentId, 1);
	}
}

void USCharacterTrainingEnvironment::GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId)
{
	OutCompletion = ELearningAgentsCompletion::Running;

	// Get the character agent
	ASCharacter* Character = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (!Character || !TargetActor)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Check if agent reached the target
	if (TargetActor->IsLocationWithinReach(Character->GetActorLocation()))
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Check if episode has exceeded maximum length
	if (EpisodeSteps.Contains(AgentId) && EpisodeSteps[AgentId] >= MaxEpisodeLength)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Check if character is outside bounds
	FVector CharacterLocation = Character->GetActorLocation();
	FVector BoundsMin = ResetCenter - ResetBounds;
	FVector BoundsMax = ResetCenter + ResetBounds;
	
	if (CharacterLocation.X < BoundsMin.X || CharacterLocation.X > BoundsMax.X ||
		CharacterLocation.Y < BoundsMin.Y || CharacterLocation.Y > BoundsMax.Y)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}

	// Check if character died
	if (Character->IsDead())
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}
}

void USCharacterTrainingEnvironment::ResetAgentEpisode_Implementation(const int32 AgentId)
{
	// Get the character agent
	ASCharacter* Character = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (!Character || !TargetActor)
	{
		return;
	}

	// Reset episode step counter
	EpisodeSteps.Add(AgentId, 0);
	PreviousDistances.Remove(AgentId);

	// Reset character to random position
	FVector CharacterResetLocation;
	do {
		CharacterResetLocation.X = ResetCenter.X + FMath::RandRange(-ResetBounds.X, ResetBounds.X);
		CharacterResetLocation.Y = ResetCenter.Y + FMath::RandRange(-ResetBounds.Y, ResetBounds.Y);
		CharacterResetLocation.Z = ResetCenter.Z + ResetBounds.Z; // Keep above ground
	} while (false); // We'll place character first, then target

	// Use the character's learning reset method
	Character->ResetForLearning(CharacterResetLocation, FRotator::ZeroRotator);

	// Reset target to random position (ensuring minimum distance from character)
	FVector TargetResetLocation;
	int32 Attempts = 0;
	do {
		TargetResetLocation.X = ResetCenter.X + FMath::RandRange(-ResetBounds.X, ResetBounds.X);
		TargetResetLocation.Y = ResetCenter.Y + FMath::RandRange(-ResetBounds.Y, ResetBounds.Y);
		TargetResetLocation.Z = ResetCenter.Z + ResetBounds.Z; // Keep above ground
		Attempts++;
	} while (FVector::Dist(CharacterResetLocation, TargetResetLocation) < MinDistanceBetweenCharacterAndTarget && Attempts < 100);

	TargetActor->SetActorLocation(TargetResetLocation);

	UE_LOG(LogTemp, Log, TEXT("Reset Agent %d - Character: %s, Target: %s, Distance: %f"), 
		AgentId, 
		*CharacterResetLocation.ToString(), 
		*TargetResetLocation.ToString(),
		FVector::Dist(CharacterResetLocation, TargetResetLocation));
} 