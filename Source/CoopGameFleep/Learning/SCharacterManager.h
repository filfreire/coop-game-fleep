// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsTrainer.h"
#include "LearningAgentsPPOTrainer.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsCommunicator.h"
#include "SCharacterManager.generated.h"

class USCharacterManagerComponent;
class USCharacterInteractor;
class USCharacterTrainingEnvironment;
class ASTargetActor;
class ULearningAgentsNeuralNetwork;

UENUM(BlueprintType)
enum class ESCharacterManagerMode : uint8
{
	Training		UMETA(DisplayName = "Training"),
	Inference		UMETA(DisplayName = "Inference"),
	ReInitialize	UMETA(DisplayName = "ReInitialize")
};

/**
 * Main manager for SCharacter learning agents
 */
UCLASS()
class COOPGAMEFLEEP_API ASCharacterManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ASCharacterManager();

protected:
	virtual void BeginPlay() override;

	// Core learning components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USCharacterManagerComponent* LearningAgentsManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsInteractor* LearningAgentsInteractorBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	USCharacterInteractor* Interactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsPolicy* Policy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsCritic* Critic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsTrainingEnvironment* TrainingEnvironmentBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	USCharacterTrainingEnvironment* TrainingEnvironment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsPPOTrainer* PPOTrainer;

	// Internal initialization functions
	void InitializeAgents();
	void InitializeManager();

public:	
	virtual void Tick(float DeltaTime) override;

	// Manager settings
	UPROPERTY(EditAnywhere, Category = "Manager Settings")
	ESCharacterManagerMode RunMode = ESCharacterManagerMode::Training;

	UPROPERTY(EditAnywhere, Category = "Manager Settings")
	int32 RandomSeed = 1234;

	// Learning settings
	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsPolicySettings PolicySettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsCriticSettings CriticSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsPPOTrainingSettings TrainingSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsTrainingGameSettings TrainingGameSettings;

	// Neural network references
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* EncoderNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* PolicyNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* DecoderNeuralNetwork;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* CriticNeuralNetwork;

	// Target actor reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	ASTargetActor* TargetActor;

	// Trainer settings - expose these to editor like in car example
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	FLearningAgentsTrainerProcessSettings TrainerProcessSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	FLearningAgentsSharedMemoryCommunicatorSettings SharedMemorySettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	FLearningAgentsPPOTrainerSettings TrainerSettings;
}; 