// Fill out your copyright notice in the Description page of Project Settings.

#include "MyLearningAgentsManager.h"
#include "MyLearningAgentsInteractor.h"
#include "MyTrainingEnvironment.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsPPOTrainer.h"
#include "LearningAgentsCommunicator.h"
#include "LearningAgentsNeuralNetwork.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"

AMyLearningAgentsManager::AMyLearningAgentsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Create the Learning Agents Manager component
	LearningAgentsManagerComponent = CreateDefaultSubobject<ULearningAgentsManager>(TEXT("Learning Agents Manager"));
	
	// Set default values
	RunMode = ECharacterManagerMode::Training;
	RandomSeed = 1234;
	TargetLocation = FVector(0.0f, 0.0f, 0.0f);
	TargetActor = nullptr;

	// Initialize to nullptr
	Interactor = nullptr;
	Policy = nullptr;
	Critic = nullptr;
	TrainingEnvironment = nullptr;
	PPOTrainer = nullptr;
	LearningAgentsInteractorBase = nullptr;
	TrainingEnvironmentBase = nullptr;

	// Neural network assets (will be set in Blueprint)
	EncoderNeuralNetwork = nullptr;
	PolicyNeuralNetwork = nullptr;
	DecoderNeuralNetwork = nullptr;
	CriticNeuralNetwork = nullptr;
}

void AMyLearningAgentsManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize agents and manager
	InitializeAgents();
	InitializeManager();
}

void AMyLearningAgentsManager::InitializeAgents()
{
	// Get all character agents in the world
	TArray<AActor*> Agents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCharacter::StaticClass(), Agents);

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Found %d character agents"), Agents.Num());

	for (AActor* Agent : Agents)
	{
		// Make sure manager ticks first
		Agent->AddTickPrerequisiteActor(this);

		// Add agent to the Learning Agents Manager
		if (LearningAgentsManagerComponent)
		{
			LearningAgentsManagerComponent->AddAgent(Agent);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Initialized %d agents"), Agents.Num());
}

void AMyLearningAgentsManager::InitializeManager()
{
	if (!LearningAgentsManagerComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: LearningAgentsManagerComponent is null"));
		return;
	}

	// Check if we should reinitialize neural networks
	const bool bReInitialize = (RunMode == ECharacterManagerMode::ReInitialize);

	// Create Interactor
	Interactor = Cast<UMyLearningAgentsInteractor>(ULearningAgentsInteractor::MakeInteractor(
		LearningAgentsManagerComponent, UMyLearningAgentsInteractor::StaticClass(), "Character Interactor"));
	
	if (!Interactor)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: Failed to create interactor"));
		return;
	}

	// Configure interactor
	Interactor->TargetLocation = TargetLocation;
	Interactor->TargetActor = TargetActor;
	LearningAgentsInteractorBase = Interactor;

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Created interactor successfully"));

	// Check neural networks
	if (!EncoderNeuralNetwork || !PolicyNeuralNetwork || !DecoderNeuralNetwork || !CriticNeuralNetwork)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyLearningAgentsManager: One or more neural networks are not set. Please assign them in the Blueprint."));
		return;
	}

	// Create Policy
	Policy = ULearningAgentsPolicy::MakePolicy(
		LearningAgentsManagerComponent,
		LearningAgentsInteractorBase,
		ULearningAgentsPolicy::StaticClass(),
		TEXT("Character Policy"),
		EncoderNeuralNetwork,
		PolicyNeuralNetwork,
		DecoderNeuralNetwork,
		bReInitialize, bReInitialize, bReInitialize,
		PolicySettings,
		RandomSeed
	);

	if (!Policy)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: Failed to create policy"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Created policy successfully"));

	// Create Critic
	Critic = ULearningAgentsCritic::MakeCritic(
		LearningAgentsManagerComponent,
		LearningAgentsInteractorBase,
		Policy,
		ULearningAgentsCritic::StaticClass(),
		TEXT("Character Critic"),
		CriticNeuralNetwork,
		bReInitialize,
		CriticSettings,
		RandomSeed
	);

	if (!Critic)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: Failed to create critic"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Created critic successfully"));

	// Create Training Environment
	TrainingEnvironment = Cast<UMyTrainingEnvironment>(ULearningAgentsTrainingEnvironment::MakeTrainingEnvironment(
		LearningAgentsManagerComponent, UMyTrainingEnvironment::StaticClass(), "Character Training Environment"));

	if (!TrainingEnvironment)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: Failed to create training environment"));
		return;
	}

	// Configure training environment
	TrainingEnvironment->TargetLocation = TargetLocation;
	TrainingEnvironment->TargetActor = TargetActor;
	TrainingEnvironmentBase = TrainingEnvironment;

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Created training environment successfully"));

	// For training mode, create PPO trainer
	if (RunMode == ECharacterManagerMode::Training)
	{
		// Create shared memory communicator for training process
		FLearningAgentsCommunicator Communicator = ULearningAgentsCommunicatorLibrary::MakeSharedMemoryTrainingProcess(
			TrainerProcessSettings, SharedMemorySettings
		);

		// Create PPO Trainer
		PPOTrainer = ULearningAgentsPPOTrainer::MakePPOTrainer(
			LearningAgentsManagerComponent,
			LearningAgentsInteractorBase,
			TrainingEnvironmentBase,
			Policy,
			Critic,
			Communicator,
			ULearningAgentsPPOTrainer::StaticClass(),
			TEXT("Character PPO Trainer"),
			TrainerSettings
		);

		if (!PPOTrainer)
		{
			UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsManager: Failed to create PPO trainer"));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Created PPO trainer successfully"));
	}

	UE_LOG(LogTemp, Log, TEXT("MyLearningAgentsManager: Manager initialization complete"));
}

void AMyLearningAgentsManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RunMode == ECharacterManagerMode::Inference)
	{
		// Run inference mode
		if (Policy)
		{
			Policy->RunInference();
		}
	}
	else if (RunMode == ECharacterManagerMode::Training)
	{
		// Run training mode
		if (PPOTrainer)
		{
			PPOTrainer->RunTraining(TrainingSettings, TrainingGameSettings, true, true);
		}
	}
}

