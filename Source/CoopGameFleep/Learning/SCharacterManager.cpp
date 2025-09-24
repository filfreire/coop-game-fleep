// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterManager.h"
#include "SCharacterManagerComponent.h"
#include "SCharacterInteractor.h"
#include "SCharacterTrainingEnvironment.h"
#include "STargetActor.h"
#include "LearningAgentsPPOTrainer.h"
#include "LearningAgentsCommunicator.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SCharacter.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "LearningAgentsController.h"
#include "LearningAgentsEntitiesManagerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASCharacterManager::ASCharacterManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Check command line arguments to determine if we're in headless training mode
	FString CommandLine = FCommandLine::Get();
	bool bIsHeadlessTraining = CommandLine.Contains(TEXT("-headless-training")) ||
	                          CommandLine.Contains(TEXT("-nullrhi")) ||
	                          CommandLine.Contains(TEXT("-unattended"));

	// Parse command line arguments for hyperparameters
	FString RandomSeedStr;
	if (FParse::Value(*CommandLine, TEXT("-RandomSeed="), RandomSeedStr))
	{
		RandomSeed = FCString::Atoi(*RandomSeedStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: RandomSeed set from command line: %d"), RandomSeed);
	}

	// Parse PPO training hyperparameters
	FString LearningRatePolicyStr;
	if (FParse::Value(*CommandLine, TEXT("-LearningRatePolicy="), LearningRatePolicyStr))
	{
		TrainingSettings.LearningRatePolicy = FCString::Atof(*LearningRatePolicyStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: LearningRatePolicy set from command line: %f"), TrainingSettings.LearningRatePolicy);
	}

	FString LearningRateCriticStr;
	if (FParse::Value(*CommandLine, TEXT("-LearningRateCritic="), LearningRateCriticStr))
	{
		TrainingSettings.LearningRateCritic = FCString::Atof(*LearningRateCriticStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: LearningRateCritic set from command line: %f"), TrainingSettings.LearningRateCritic);
	}

	FString EpsilonClipStr;
	if (FParse::Value(*CommandLine, TEXT("-EpsilonClip="), EpsilonClipStr))
	{
		TrainingSettings.EpsilonClip = FCString::Atof(*EpsilonClipStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: EpsilonClip set from command line: %f"), TrainingSettings.EpsilonClip);
	}

	FString PolicyBatchSizeStr;
	if (FParse::Value(*CommandLine, TEXT("-PolicyBatchSize="), PolicyBatchSizeStr))
	{
		TrainingSettings.PolicyBatchSize = FCString::Atoi(*PolicyBatchSizeStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: PolicyBatchSize set from command line: %d"), TrainingSettings.PolicyBatchSize);
	}

	FString CriticBatchSizeStr;
	if (FParse::Value(*CommandLine, TEXT("-CriticBatchSize="), CriticBatchSizeStr))
	{
		TrainingSettings.CriticBatchSize = FCString::Atoi(*CriticBatchSizeStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: CriticBatchSize set from command line: %d"), TrainingSettings.CriticBatchSize);
	}

	FString IterationsPerGatherStr;
	if (FParse::Value(*CommandLine, TEXT("-IterationsPerGather="), IterationsPerGatherStr))
	{
		TrainingSettings.IterationsPerGather = FCString::Atoi(*IterationsPerGatherStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: IterationsPerGather set from command line: %d"), TrainingSettings.IterationsPerGather);
	}

	FString NumberOfIterationsStr;
	if (FParse::Value(*CommandLine, TEXT("-NumberOfIterations="), NumberOfIterationsStr))
	{
		TrainingSettings.NumberOfIterations = FCString::Atoi(*NumberOfIterationsStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: NumberOfIterations set from command line: %d"), TrainingSettings.NumberOfIterations);
	}

	FString DiscountFactorStr;
	if (FParse::Value(*CommandLine, TEXT("-DiscountFactor="), DiscountFactorStr))
	{
		TrainingSettings.DiscountFactor = FCString::Atof(*DiscountFactorStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: DiscountFactor set from command line: %f"), TrainingSettings.DiscountFactor);
	}

	FString GaeLambdaStr;
	if (FParse::Value(*CommandLine, TEXT("-GaeLambda="), GaeLambdaStr))
	{
		TrainingSettings.GaeLambda = FCString::Atof(*GaeLambdaStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: GaeLambda set from command line: %f"), TrainingSettings.GaeLambda);
	}

	FString ActionEntropyWeightStr;
	if (FParse::Value(*CommandLine, TEXT("-ActionEntropyWeight="), ActionEntropyWeightStr))
	{
		TrainingSettings.ActionEntropyWeight = FCString::Atof(*ActionEntropyWeightStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: ActionEntropyWeight set from command line: %f"), TrainingSettings.ActionEntropyWeight);
	}

	// Parse obstacle configuration parameters
	FString UseObstaclesStr;
	if (FParse::Value(*CommandLine, TEXT("-UseObstacles="), UseObstaclesStr))
	{
		ObstacleConfig.bUseObstacles = UseObstaclesStr.ToBool();
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: UseObstacles set from command line: %s"), ObstacleConfig.bUseObstacles ? TEXT("true") : TEXT("false"));
	}

	FString MaxObstaclesStr;
	if (FParse::Value(*CommandLine, TEXT("-MaxObstacles="), MaxObstaclesStr))
	{
		ObstacleConfig.MaxObstacles = FCString::Atoi(*MaxObstaclesStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: MaxObstacles set from command line: %d"), ObstacleConfig.MaxObstacles);
	}

	FString MinObstacleSizeStr;
	if (FParse::Value(*CommandLine, TEXT("-MinObstacleSize="), MinObstacleSizeStr))
	{
		ObstacleConfig.MinObstacleSize = FCString::Atof(*MinObstacleSizeStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: MinObstacleSize set from command line: %f"), ObstacleConfig.MinObstacleSize);
	}

	FString MaxObstacleSizeStr;
	if (FParse::Value(*CommandLine, TEXT("-MaxObstacleSize="), MaxObstacleSizeStr))
	{
		ObstacleConfig.MaxObstacleSize = FCString::Atof(*MaxObstacleSizeStr);
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: MaxObstacleSize set from command line: %f"), ObstacleConfig.MaxObstacleSize);
	}

	FString ObstacleModeStr;
	if (FParse::Value(*CommandLine, TEXT("-ObstacleMode="), ObstacleModeStr))
	{
		if (ObstacleModeStr.Equals(TEXT("Dynamic"), ESearchCase::IgnoreCase))
		{
			ObstacleConfig.ObstacleMode = EObstacleMode::Dynamic;
		}
		else
		{
			ObstacleConfig.ObstacleMode = EObstacleMode::Static;
		}
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: ObstacleMode set from command line: %s"), ObstacleModeStr.Equals(TEXT("Dynamic"), ESearchCase::IgnoreCase) ? TEXT("Dynamic") : TEXT("Static"));
	}

	// Only force ReInitialize mode for headless training to ensure fresh neural network initialization
	if (bIsHeadlessTraining)
	{
		RunMode = ESCharacterManagerMode::ReInitialize;
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Headless training detected, forcing RunMode to ReInitialize: %d"), (int32)RunMode);
		
	}
	else
	{
		// In editor mode, use Blueprint default (Training) but allow override
		RunMode = ESCharacterManagerMode::Training;
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Editor mode detected, using Blueprint RunMode: %d"), (int32)RunMode);
	}

	// set training settings for headless training
	TrainingSettings.bUseTensorboard = true;
	TrainingSettings.bSaveSnapshots = true;

	// Configure trainer process settings for headless training
	// These paths are relative to the packaged executable location
	// Auto-detect engine path based on hostname (same logic as packaging script)
	FString HostName = FPlatformProcess::ComputerName();
	FString EnginePath;
	
	// Platform-specific path detection
#if PLATFORM_WINDOWS
	// For headless training, we need to use the actual Engine installation
	// Use absolute paths since relative paths with drive letters are problematic
	if (HostName == TEXT("filfreire01"))
	{
		EnginePath = TEXT("C:/unreal/UE_5.6/Engine");
	}
	else if (HostName == TEXT("filfreire02"))
	{
		EnginePath = TEXT("D:/unreal/UE_5.6/Engine");
	}
	else
	{
		// Try to find Unreal Engine in typical installation locations
		FString ProgramFiles = FPlatformMisc::GetEnvironmentVariable(TEXT("ProgramFiles"));
		FString ProgramFilesX86 = FPlatformMisc::GetEnvironmentVariable(TEXT("ProgramFiles(x86)"));
		
		// Check common Unreal Engine installation paths
		TArray<FString> PossiblePaths = {
			TEXT("C:/Program Files/Epic Games/UE_5.6/Engine"),
			TEXT("C:/Program Files (x86)/Epic Games/UE_5.6/Engine"),
			TEXT("C:/unreal/UE_5.6/Engine"),
			TEXT("D:/unreal/UE_5.6/Engine"),
			ProgramFiles + TEXT("/Epic Games/UE_5.6/Engine"),
			ProgramFilesX86 + TEXT("/Epic Games/UE_5.6/Engine")
		};
		
		// Find the first existing path
		bool bFoundPath = false;
		for (const FString& Path : PossiblePaths)
		{
			if (FPaths::DirectoryExists(Path))
			{
				EnginePath = Path;
				bFoundPath = true;
				UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Found Unreal Engine at: %s"), *EnginePath);
				break;
			}
		}
		
		// Final fallback if no path found
		if (!bFoundPath)
		{
			EnginePath = TEXT("C:/Program Files/Epic Games/UE_5.6/Engine");
			UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Unreal Engine not found in common locations, using default: %s"), *EnginePath);
		}
	}
#elif PLATFORM_LINUX
	// Linux paths - adjust as needed for your installation
	if (HostName == TEXT("filfreire01"))
	{
		EnginePath = TEXT("/opt/unreal/UE_5.6/Engine");
	}
	else if (HostName == TEXT("filfreire02"))
	{
		EnginePath = TEXT("/opt/unreal/UE_5.6/Engine");
	}
	else
	{
		// Default fallback for Linux
		EnginePath = TEXT("/opt/unreal/UE_5.6/Engine");
	}
#else
	// Other platforms - use default Linux path
	EnginePath = TEXT("/opt/unreal/UE_5.6/Engine");
#endif

	TrainerProcessSettings.NonEditorEngineRelativePath = EnginePath;
	TrainerProcessSettings.NonEditorIntermediateRelativePath = TEXT("../../../../../Intermediate");

	// Log the configured paths for debugging
	UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Configured trainer paths for hostname '%s':"), *HostName);
	UE_LOG(LogTemp, Log, TEXT("  Engine Path: %s"), *TrainerProcessSettings.NonEditorEngineRelativePath);
	UE_LOG(LogTemp, Log, TEXT("  Intermediate Path: %s"), *TrainerProcessSettings.NonEditorIntermediateRelativePath);

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
	// Get all SCharacter agents (including Blueprint-derived ones)
	TArray<AActor*> Agents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASCharacter::StaticClass(), Agents);

	// Also try to find any Character-derived actors that might be blueprints
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), AllCharacters);

	// Filter characters to only include SCharacter and its derivatives
	for (AActor* Actor : AllCharacters)
	{
		if (ASCharacter* SChar = Cast<ASCharacter>(Actor))
		{
			if (!Agents.Contains(Actor)) // Avoid duplicates
			{
				Agents.Add(Actor);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Found %d total characters in world"), AllCharacters.Num());
	UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Found %d SCharacter agents"), Agents.Num());

	// Log details about what we found
	for (int32 i = 0; i < AllCharacters.Num(); i++)
	{
		AActor* Actor = AllCharacters[i];
		ASCharacter* SChar = Cast<ASCharacter>(Actor);
		UE_LOG(LogTemp, Warning, TEXT("Character %d: %s (Class: %s) - SCharacter Cast: %s"),
			i,
			*Actor->GetName(),
			*Actor->GetClass()->GetName(),
			SChar ? TEXT("SUCCESS") : TEXT("FAILED"));
	}

	for (AActor* Agent : Agents)
	{
		// Ensure the agent has a controller for movement input
		if (APawn* Pawn = Cast<APawn>(Agent))
		{
			AController* ExistingController = Pawn->GetController();
			if (!ExistingController)
			{
				// Create an AI controller for learning agents
				UWorld* World = GetWorld();
				if (World)
				{
					AAIController* NewController = World->SpawnActor<AAIController>();
					if (NewController)
					{
						NewController->Possess(Pawn);
						UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Created AIController for agent %s"), *Agent->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("SCharacterManager: Failed to create AIController for agent %s"), *Agent->GetName());
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Agent %s already has controller %s"),
					*Agent->GetName(), *ExistingController->GetClass()->GetName());
			}
		}

		// Add agent to the Learning Agents Manager
		int32 AgentId = LearningAgentsManager->AddAgent(Agent);
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Added agent %s to manager with ID %d"), *Agent->GetName(), AgentId);

		// Initialize agent for learning (disable player input, prepare for AI control)
		if (ASCharacter* SChar = Cast<ASCharacter>(Agent))
		{
			SChar->ResetForLearning(SChar->GetActorLocation(), SChar->GetActorRotation());
			UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Initialized %s for learning"), *Agent->GetName());
		}

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

	if (Agents.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterManager: No SCharacter agents found! Make sure to:"));
		UE_LOG(LogTemp, Error, TEXT("1. Place SCharacter (or Blueprint derived from SCharacter) actors in your level"));
		UE_LOG(LogTemp, Error, TEXT("2. Don't just set SCharacter as PlayerPawn - you need actual actors in the world"));
		UE_LOG(LogTemp, Error, TEXT("3. Check that your Blueprint inherits from SCharacter, not just Character"));
	}
}

void ASCharacterManager::InitializeManager()
{
	UE_LOG(LogTemp, Log, TEXT("SCharacterManager: InitializeManager called with RunMode: %d"), (int32)RunMode);

	// Check if we're in headless training mode and force Training if needed
	FString CommandLine = FCommandLine::Get();
	bool bIsHeadlessTraining = CommandLine.Contains(TEXT("-headless-training")) ||
	                          CommandLine.Contains(TEXT("-nullrhi")) ||
	                          CommandLine.Contains(TEXT("-unattended"));

	// Only force ReInitialize mode for headless training, respect Blueprint settings in editor
	if (bIsHeadlessTraining && RunMode != ESCharacterManagerMode::ReInitialize)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Headless training detected, forcing RunMode from %d to ReInitialize"), (int32)RunMode);
		RunMode = ESCharacterManagerMode::ReInitialize;
	}
	else if (!bIsHeadlessTraining)
	{
		UE_LOG(LogTemp, Log, TEXT("SCharacterManager: Editor mode - respecting Blueprint RunMode: %d"), (int32)RunMode);
	}

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
		ManagerPtr, InteractorPtr, ULearningAgentsPolicy::StaticClass(), TEXT("SCharacter Policy"),
		EncoderNeuralNetwork, PolicyNeuralNetwork, DecoderNeuralNetwork,
		ReInitialize, ReInitialize, ReInitialize, PolicySettings, RandomSeed);
	if (Policy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SCharacterManager: Failed to make policy object."));
		return;
	}

	// Make Critic Instance
	ULearningAgentsPolicy* PolicyPtr = Policy;
	Critic = ULearningAgentsCritic::MakeCritic(
		ManagerPtr, InteractorPtr, PolicyPtr, ULearningAgentsCritic::StaticClass(), TEXT("SCharacter Critic"),
		CriticNeuralNetwork, ReInitialize, CriticSettings, RandomSeed);
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
	
	// Configure obstacles from command line parameters
	TrainingEnvironment->ConfigureObstacles(
		ObstacleConfig.bUseObstacles,
		ObstacleConfig.MaxObstacles,
		ObstacleConfig.MinObstacleSize,
		ObstacleConfig.MaxObstacleSize,
		ObstacleConfig.ObstacleMode
	);
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