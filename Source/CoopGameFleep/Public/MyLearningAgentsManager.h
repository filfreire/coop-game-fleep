// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsPPOTrainer.h"
#include "LearningAgentsCommunicator.h"
#include "Engine/World.h"
#include "MyLearningAgentsManager.generated.h"

class UMyLearningAgentsInteractor;
class UMyTrainingEnvironment;
class ULearningAgentsPolicy;
class ULearningAgentsCritic;
class ULearningAgentsPPOTrainer;
class ULearningAgentsNeuralNetwork;

UENUM(BlueprintType)
enum class ECharacterManagerMode : uint8
{
	Training		UMETA(DisplayName = "Training"),
	Inference		UMETA(DisplayName = "Inference"),
	ReInitialize	UMETA(DisplayName = "ReInitialize")
};

/**
 * Manager for character-based Learning Agents setup
 */
UCLASS(BlueprintType, Blueprintable)
class COOPGAMEFLEEP_API AMyLearningAgentsManager : public AActor
{
	GENERATED_BODY()

public:
	AMyLearningAgentsManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// Manager component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	ULearningAgentsManager* LearningAgentsManagerComponent;

	// Learning objects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	UMyLearningAgentsInteractor* Interactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsPolicy* Policy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsCritic* Critic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	UMyTrainingEnvironment* TrainingEnvironment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsPPOTrainer* PPOTrainer;

	// Base pointers for Learning Agents framework
	UPROPERTY()
	ULearningAgentsInteractor* LearningAgentsInteractorBase;

	UPROPERTY()
	ULearningAgentsTrainingEnvironment* TrainingEnvironmentBase;

	// Settings structures
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Settings")
	FLearningAgentsTrainerProcessSettings TrainerProcessSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Settings")
	FLearningAgentsSharedMemoryCommunicatorSettings SharedMemorySettings;

public:
	// Manager settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
	ECharacterManagerMode RunMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
	int32 RandomSeed;

	// Target configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TSoftObjectPtr<AActor> TargetActor;

	// Learning settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
	FLearningAgentsPolicySettings PolicySettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
	FLearningAgentsCriticSettings CriticSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
	FLearningAgentsPPOTrainerSettings TrainerSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
	FLearningAgentsPPOTrainingSettings TrainingSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
	FLearningAgentsTrainingGameSettings TrainingGameSettings;

	// Neural network assets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* EncoderNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* PolicyNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* DecoderNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* CriticNeuralNetwork;

protected:
	// Initialization functions
	void InitializeAgents();
	void InitializeManager();
};
