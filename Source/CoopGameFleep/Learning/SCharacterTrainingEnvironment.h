// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Learning/ObstacleTypes.h"
#include "LearningAgentsTrainingEnvironment.h"
#include "SCharacterTrainingEnvironment.generated.h"

class ASTargetActor;
class USObstacleManager;

/**
 * Training environment for SCharacter learning to move to target
 */
UCLASS()
class COOPGAMEFLEEP_API USCharacterTrainingEnvironment : public ULearningAgentsTrainingEnvironment
{
	GENERATED_BODY()

  public:
	USCharacterTrainingEnvironment();

	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;
	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion,
	                                                  const int32 AgentId) override;
	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

	// Target actor reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	ASTargetActor* TargetActor;

	// Obstacle manager for handling obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	USObstacleManager* ObstacleManager;

	// Reward settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float ReachTargetReward = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float DistanceRewardScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float MovementTowardsTargetReward = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float FacingTargetReward = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float TimeStepPenalty = -0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float MaxEpisodeLength = 1000.0f;

	// Reset bounds for character and target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FVector ResetCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FVector ResetBounds = FVector(2000.0f, 2000.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float MinDistanceBetweenCharacterAndTarget = 500.0f;

	// Additional clearance above the reset center to prevent floor clipping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float GroundClearance = 200.0f;

	// Obstacle configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	bool bUseObstacles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	int32 MaxObstacles = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	float MinObstacleSize = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	float MaxObstacleSize = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
	EObstacleMode ObstacleMode = EObstacleMode::Static;

	// Function to configure obstacles from external source
	UFUNCTION(BlueprintCallable, Category = "Obstacles")
	void ConfigureObstacles(bool bUse, int32 MaxObs, float MinSize, float MaxSize, EObstacleMode Mode);

  private:
	// Store previous distances for reward calculation
	TMap<int32, float> PreviousDistances;
	TMap<int32, int32> EpisodeSteps;
};
