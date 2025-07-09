// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainingEnvironment.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "MyTrainingEnvironment.generated.h"

class ASCharacter;

/**
 * Training environment for character movement and goal reaching
 */
UCLASS()
class COOPGAMEFLEEP_API UMyTrainingEnvironment : public ULearningAgentsTrainingEnvironment
{
	GENERATED_BODY()

public:
	UMyTrainingEnvironment();

	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;
	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId) override;
	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

	// Target location that agents should reach
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FVector TargetLocation;

	// Alternative: Use an actor as target (more flexible)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TSoftObjectPtr<AActor> TargetActor;

	// Rewards configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards", meta = (ClampMin = "10.0"))
	float GoalReachDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards", meta = (ClampMin = "5.0"))
	float MaxEpisodeTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float MovementRewardScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float GoalRewardAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float TimePenaltyPerSecond;

	// Reset configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reset")
	TArray<FVector> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reset", meta = (ClampMin = "100.0"))
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reset")
	float MinDistanceBetweenAgents;

private:
	// Track episode start times for timeout
	TMap<int32, float> EpisodeStartTimes;
	
	// Helper function to find nearest spawn point with no collisions
	FVector GetValidSpawnLocation(const TArray<AActor*>& AllAgents) const;
};
