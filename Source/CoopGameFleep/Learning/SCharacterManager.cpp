// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterManager.h"
#include "SCharacterManagerComponent.h"
#include "SCharacterInteractor.h"
#include "SCharacterTrainingEnvironment.h"
#include "STargetActor.h"
#include "LearningAgentsPPOTrainer.h"
#include "LearningAgentsCommunicator.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"

ASCharacterManager::ASCharacterManager()
{
	PrimaryActorTick.bCanEverTick = true;

	LearningAgentsManager = CreateDefaultSubobject<USCharacterManagerComponent>(TEXT("Learning Agents Manager"));
}

void ASCharacterManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the learning system
	InitializeAgents();
	InitializeManager();
}

void ASCharacterManager::InitializeAgents()
{
	// Get all SCharacter agents
	TArray<AActor*> Agents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCharacter::StaticClass(), Agents);

	for (AActor* Agent : Agents)
	{
		// Make sure manager ticks first
		Agent->AddTickPrerequisiteActor(this);

		// If in inference mode, we could reset positions here if needed
		if (RunMode == ESCharacterManagerMode::Inference)
		{
			// For now, just log that we're in inference mode
			UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Agent %s initialized in inference mode"), *Agent->GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Initialized %d character agents"), Agents.Num());
}

void ASCharacterManager::InitializeManager()
{
	// Should neural networks be re-initialized
	const bool ReInitialize = (RunMode == ESCharacterManagerMode::ReInitialize);

	// Make Interactor Instance
	ULearningAgentsManager* ManagerPtr = LearningAgentsManager;
	Interactor = Cast<USCharacterInteractor>(ULearningAgentsInteractor::MakeInteractor(
		ManagerPtr, USCharacterInteractor::StaticClass(), TEXT("SCharacter Interactor")));
	if (Interactor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make interactor object."));
		return;
	}
	Interactor->TargetActor = TargetActor;
	LearningAgentsInteractorBase = Interactor;

	// Warn if neural networks are not set
	if (EncoderNeuralNetwork == nullptr || PolicyNeuralNetwork == nullptr || 
		DecoderNeuralNetwork == nullptr || CriticNeuralNetwork == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: One or more neural networks are not set."));
		return;
	}

	// Make Policy Instance
	ULearningAgentsInteractor* InteractorPtr = Interactor;
	Policy = ULearningAgentsPolicy::MakePolicy(
		ManagerPtr, InteractorPtr, USCharacterInteractor::StaticClass(), TEXT("SCharacter Policy"), 
		EncoderNeuralNetwork, PolicyNeuralNetwork, DecoderNeuralNetwork, 
		ReInitialize, ReInitialize, ReInitialize, PolicySettings);
	if (Policy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make policy object."));
		return;
	}

	// Make Critic Instance
	ULearningAgentsPolicy* PolicyPtr = Policy;
	Critic = ULearningAgentsCritic::MakeCritic(
		ManagerPtr, InteractorPtr, PolicyPtr, USCharacterInteractor::StaticClass(), TEXT("SCharacter Critic"), 
		CriticNeuralNetwork, ReInitialize, CriticSettings);
	if (Critic == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make critic object."));
		return;
	}

	// Make Training Environment Instance
	TrainingEnvironment = Cast<USCharacterTrainingEnvironment>(ULearningAgentsTrainingEnvironment::MakeTrainingEnvironment(
		ManagerPtr, USCharacterTrainingEnvironment::StaticClass(), TEXT("SCharacter Training Environment")));
	if (TrainingEnvironment == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make training environment object."));
		return;
	}
	TrainingEnvironment->TargetActor = TargetActor;
	TrainingEnvironmentBase = TrainingEnvironment;

	// Create a shared memory communicator to spawn a training process (following car example)
	FLearningAgentsCommunicator Communicator = ULearningAgentsCommunicatorLibrary::MakeSharedMemoryTrainingProcess(
		TrainerProcessSettings, SharedMemorySettings
	);

	// Make PPO Trainer Instance
	PPOTrainer = ULearningAgentsPPOTrainer::MakePPOTrainer(
		ManagerPtr, InteractorPtr, TrainingEnvironmentBase, Policy, Critic,
		Communicator, ULearningAgentsPPOTrainer::StaticClass(), TEXT("SCharacter PPO Trainer"), TrainerSettings);
	if (PPOTrainer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make PPO trainer object."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Initialization complete. Mode: %d"), (int32)RunMode);
}

void ASCharacterManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle different run modes like in car example
	if (RunMode == ESCharacterManagerMode::Inference)
	{
		if (Policy != nullptr)
		{
			Policy->RunInference();
		}
	}
	else // Training or ReInitialize mode
	{
		if (PPOTrainer != nullptr)
		{
			PPOTrainer->RunTraining(TrainingSettings, TrainingGameSettings, true, true);
		}
	}
} 