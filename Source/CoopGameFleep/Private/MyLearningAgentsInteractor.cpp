// Fill out your copyright notice in the Description page of Project Settings.

#include "MyLearningAgentsInteractor.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsActions.h"
#include "LearningAgentsManager.h"
#include "SCharacter.h"
#include "Components/SHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

typedef TPair<float, AActor*> FCharacterMeasurement;

UMyLearningAgentsInteractor::UMyLearningAgentsInteractor()
{
	TargetLocation = FVector::ZeroVector;
	TargetActor = nullptr;
	OtherAgentCount = 5;
	ObservationDistance = 5000.0f;
	MaxMovementSpeed = 600.0f;
}

void UMyLearningAgentsInteractor::SpecifyAgentObservation_Implementation(
	FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
	ULearningAgentsObservationSchema* InObservationSchema)
{
	// Define target observation
	TMap<FName, FLearningAgentsObservationSchemaElement> TargetObservation;
	TargetObservation.Add("TargetLocation", ULearningAgentsObservations::SpecifyLocationObservation(
		InObservationSchema, ObservationDistance, "TargetLocationObservation"));
	TargetObservation.Add("TargetDirection", ULearningAgentsObservations::SpecifyDirectionObservation(
		InObservationSchema, "TargetDirectionObservation"));
	TargetObservation.Add("DistanceToTarget", ULearningAgentsObservations::SpecifyFloatObservation(
		InObservationSchema, ObservationDistance, "DistanceToTargetObservation"));

	// Define other agent observations
	TMap<FName, FLearningAgentsObservationSchemaElement> AgentObservation;
	AgentObservation.Add("Location", ULearningAgentsObservations::SpecifyLocationObservation(
		InObservationSchema, ObservationDistance, "AgentLocationObservation"));
	AgentObservation.Add("Direction", ULearningAgentsObservations::SpecifyDirectionObservation(
		InObservationSchema, "AgentDirectionObservation"));
	AgentObservation.Add("Distance", ULearningAgentsObservations::SpecifyFloatObservation(
		InObservationSchema, ObservationDistance, "AgentDistanceObservation"));

	// Wrap agents in static array
	const auto AgentArrayObservations = ULearningAgentsObservations::SpecifyStaticArrayObservation(
		InObservationSchema,
		ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, AgentObservation),
		OtherAgentCount);

	// Define self observations
	TMap<FName, FLearningAgentsObservationSchemaElement> SelfObservation;
	SelfObservation.Add("Location", ULearningAgentsObservations::SpecifyLocationObservation(
		InObservationSchema, ObservationDistance, "SelfLocationObservation"));
	SelfObservation.Add("Velocity", ULearningAgentsObservations::SpecifyVelocityObservation(
		InObservationSchema, MaxMovementSpeed, "SelfVelocityObservation"));
	SelfObservation.Add("Rotation", ULearningAgentsObservations::SpecifyRotationObservation(
		InObservationSchema, "SelfRotationObservation"));
	SelfObservation.Add("Health", ULearningAgentsObservations::SpecifyFloatObservation(
		InObservationSchema, 100.0f, "SelfHealthObservation"));

	// Pack everything into a single struct
	TMap<FName, FLearningAgentsObservationSchemaElement> OutObservationMap;
	OutObservationMap.Add("Target", ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, TargetObservation));
	OutObservationMap.Add("OtherAgents", AgentArrayObservations);
	OutObservationMap.Add("Self", ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, SelfObservation));

	// Set to outward bound schema element
	OutObservationSchemaElement = ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, OutObservationMap);
}

void UMyLearningAgentsInteractor::GatherAgentObservation_Implementation(
	FLearningAgentsObservationObjectElement& OutObservationObjectElement,
	ULearningAgentsObservationObject* InObservationObject, const int32 AgentId)
{
	// Get reference to the character agent
	ASCharacter* Agent = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (Agent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsInteractor: Failed to cast agent to ASCharacter"));
		return;
	}

	const FVector AgentLocation = Agent->GetActorLocation();
	const FTransform AgentTransform = Agent->GetActorTransform();

	// Determine current target location
	FVector CurrentTargetLocation = TargetLocation;
	if (TargetActor.IsValid())
	{
		CurrentTargetLocation = TargetActor->GetActorLocation();
	}

	// Make target observations
	const FVector DirectionToTarget = (CurrentTargetLocation - AgentLocation).GetSafeNormal();
	const float DistanceToTarget = FVector::Dist(AgentLocation, CurrentTargetLocation);

	TMap<FName, FLearningAgentsObservationObjectElement> TargetObservations;
	TargetObservations.Add("TargetLocation", ULearningAgentsObservations::MakeLocationObservation(
		InObservationObject, CurrentTargetLocation, AgentTransform, "TargetLocationObservation"));
	TargetObservations.Add("TargetDirection", ULearningAgentsObservations::MakeDirectionObservation(
		InObservationObject, DirectionToTarget, AgentTransform, "TargetDirectionObservation"));
	TargetObservations.Add("DistanceToTarget", ULearningAgentsObservations::MakeFloatObservation(
		InObservationObject, DistanceToTarget, "DistanceToTargetObservation"));

	// Gather observations of other agents
	TArray<FCharacterMeasurement> Measurements;
	TArray<UObject*> AllAgents;
	TArray<int32> AllIDs;

	// Get all agents and measure distances
	Manager->GetAllAgents(AllAgents, AllIDs, ASCharacter::StaticClass());
	for (UObject* AgentObject : AllAgents)
	{
		AActor* OtherAgent = Cast<AActor>(AgentObject);
		if (OtherAgent != nullptr && OtherAgent != Agent)
		{
			float DistToAgent = FVector::Dist(AgentLocation, OtherAgent->GetActorLocation());
			Measurements.Add(FCharacterMeasurement(DistToAgent, OtherAgent));
		}
	}

	// Sort by distance and pick the closest agents
	Algo::SortBy(Measurements, &FCharacterMeasurement::Key);
	TArray<FLearningAgentsObservationObjectElement> AgentObservations;
	AgentObservations.Reserve(OtherAgentCount);

	for (int32 i = 0; i < OtherAgentCount; i++)
	{
		if (i < Measurements.Num())
		{
			const AActor* OtherAgent = Measurements[i].Value;
			const FVector OtherAgentLocation = OtherAgent->GetActorLocation();
			const FVector DirectionToAgent = (OtherAgentLocation - AgentLocation).GetSafeNormal();
			const float DistanceToAgent = Measurements[i].Key;

			TMap<FName, FLearningAgentsObservationObjectElement> AgentObservation;
			AgentObservation.Add("Location", ULearningAgentsObservations::MakeLocationObservation(
				InObservationObject, OtherAgentLocation, AgentTransform, "AgentLocationObservation"));
			AgentObservation.Add("Direction", ULearningAgentsObservations::MakeDirectionObservation(
				InObservationObject, DirectionToAgent, AgentTransform, "AgentDirectionObservation"));
			AgentObservation.Add("Distance", ULearningAgentsObservations::MakeFloatObservation(
				InObservationObject, DistanceToAgent, "AgentDistanceObservation"));

			AgentObservations.Add(ULearningAgentsObservations::MakeStructObservation(InObservationObject, AgentObservation));
		}
		else
		{
			// Fill with default values if not enough agents
			TMap<FName, FLearningAgentsObservationObjectElement> AgentObservation;
			AgentObservation.Add("Location", ULearningAgentsObservations::MakeLocationObservation(
				InObservationObject, FVector::ZeroVector, AgentTransform, "AgentLocationObservation"));
			AgentObservation.Add("Direction", ULearningAgentsObservations::MakeDirectionObservation(
				InObservationObject, FVector::ForwardVector, AgentTransform, "AgentDirectionObservation"));
			AgentObservation.Add("Distance", ULearningAgentsObservations::MakeFloatObservation(
				InObservationObject, ObservationDistance, "AgentDistanceObservation"));

			AgentObservations.Add(ULearningAgentsObservations::MakeStructObservation(InObservationObject, AgentObservation));
		}
	}

	FLearningAgentsObservationObjectElement AgentObservationsArray =
		ULearningAgentsObservations::MakeStaticArrayObservation(InObservationObject, AgentObservations);

	// Make self observations
	const UCharacterMovementComponent* MovementComponent = Agent->GetCharacterMovement();
	const FVector Velocity = MovementComponent ? MovementComponent->Velocity : FVector::ZeroVector;
	const FRotator Rotation = Agent->GetActorRotation();
	
	// Get health - assuming health component exists (modify based on your character implementation)
	float Health = 100.0f; // Default value
	USHealthComponent* HealthComp = Agent->GetHealthComponent();
	if (HealthComp)
	{
		// Assuming your health component has a GetHealth method - adjust as needed
		Health = HealthComp->GetHealth();
	}

	TMap<FName, FLearningAgentsObservationObjectElement> SelfObservations;
	SelfObservations.Add("Location", ULearningAgentsObservations::MakeLocationObservation(
		InObservationObject, AgentLocation, FTransform::Identity, "SelfLocationObservation"));
	SelfObservations.Add("Velocity", ULearningAgentsObservations::MakeVelocityObservation(
		InObservationObject, Velocity, AgentTransform, "SelfVelocityObservation"));
	SelfObservations.Add("Rotation", ULearningAgentsObservations::MakeRotationObservation(
		InObservationObject, Rotation, FRotator::ZeroRotator, "SelfRotationObservation"));
	SelfObservations.Add("Health", ULearningAgentsObservations::MakeFloatObservation(
		InObservationObject, Health, "SelfHealthObservation"));

	// Put observations together into a single struct
	TMap<FName, FLearningAgentsObservationObjectElement> ObservationMap;
	ObservationMap.Add("Target", ULearningAgentsObservations::MakeStructObservation(InObservationObject, TargetObservations));
	ObservationMap.Add("OtherAgents", AgentObservationsArray);
	ObservationMap.Add("Self", ULearningAgentsObservations::MakeStructObservation(InObservationObject, SelfObservations));

	// Assign to outward bound observation element
	OutObservationObjectElement = ULearningAgentsObservations::MakeStructObservation(InObservationObject, ObservationMap);
}

void UMyLearningAgentsInteractor::SpecifyAgentAction_Implementation(
	FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema)
{
	// Build map of actions for character movement
	TMap<FName, FLearningAgentsActionSchemaElement> ActionsMap;
	
	// Movement actions (normalized -1 to 1)
	ActionsMap.Add("MoveForward", ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "MoveForward"));
	ActionsMap.Add("MoveRight", ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "MoveRight"));
	
	// Rotation action (normalized -1 to 1 for left/right turning)
	ActionsMap.Add("Turn", ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "Turn"));
	
	// Optional: Jump action (boolean)
	ActionsMap.Add("Jump", ULearningAgentsActions::SpecifyBoolAction(InActionSchema, 0.5f, TEXT("Jump")));

	// Optional: Crouch action (boolean) 
	ActionsMap.Add("Crouch", ULearningAgentsActions::SpecifyBoolAction(InActionSchema, 0.5f, TEXT("Crouch")));

	// Set to outward bound struct element
	OutActionSchemaElement = ULearningAgentsActions::SpecifyStructAction(InActionSchema, ActionsMap);
}

void UMyLearningAgentsInteractor::PerformAgentAction_Implementation(
	const ULearningAgentsActionObject* InActionObject,
	const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId)
{
	// Get reference to the character agent
	ASCharacter* Agent = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (Agent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsInteractor: Failed to cast agent to ASCharacter"));
		return;
	}

	// Extract the action map
	TMap<FName, FLearningAgentsActionObjectElement> ActionMap;
	if (!ULearningAgentsActions::GetStructAction(ActionMap, InActionObject, InActionObjectElement))
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsInteractor: Failed to get struct action"));
		return;
	}

	// Get individual action elements
	const FLearningAgentsActionObjectElement* MoveForwardAction = ActionMap.Find("MoveForward");
	const FLearningAgentsActionObjectElement* MoveRightAction = ActionMap.Find("MoveRight");
	const FLearningAgentsActionObjectElement* TurnAction = ActionMap.Find("Turn");
	const FLearningAgentsActionObjectElement* JumpAction = ActionMap.Find("Jump");
	const FLearningAgentsActionObjectElement* CrouchAction = ActionMap.Find("Crouch");

	if (!MoveForwardAction || !MoveRightAction || !TurnAction || !JumpAction || !CrouchAction)
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsInteractor: Missing required actions"));
		return;
	}

	// Get action values
	float MoveForwardValue = 0.0f;
	float MoveRightValue = 0.0f;
	float TurnValue = 0.0f;
	bool bJumpValue = false;
	bool bCrouchValue = false;

	if (!ULearningAgentsActions::GetFloatAction(MoveForwardValue, InActionObject, *MoveForwardAction, "MoveForward") ||
		!ULearningAgentsActions::GetFloatAction(MoveRightValue, InActionObject, *MoveRightAction, "MoveRight") ||
		!ULearningAgentsActions::GetFloatAction(TurnValue, InActionObject, *TurnAction, "Turn") ||
		!ULearningAgentsActions::GetBoolAction(bJumpValue, InActionObject, *JumpAction, "Jump") ||
		!ULearningAgentsActions::GetBoolAction(bCrouchValue, InActionObject, *CrouchAction, "Crouch"))
	{
		UE_LOG(LogTemp, Error, TEXT("MyLearningAgentsInteractor: Failed to retrieve action values"));
		return;
	}

	// Apply movement actions to the character
	if (FMath::Abs(MoveForwardValue) > 0.01f)
	{
		Agent->AIMoveForward(MoveForwardValue);
	}

	if (FMath::Abs(MoveRightValue) > 0.01f)
	{
		Agent->AIMoveRight(MoveRightValue);
	}

	// Apply rotation
	if (FMath::Abs(TurnValue) > 0.01f)
	{
		Agent->AddControllerYawInput(TurnValue);
	}

	// Apply jump
	if (bJumpValue)
	{
		Agent->Jump();
	}

	// Apply crouch
	if (bCrouchValue)
	{
		if (!Agent->GetCharacterMovement()->IsCrouching())
		{
			Agent->AIBeginCrouch();
		}
	}
	else
	{
		if (Agent->GetCharacterMovement()->IsCrouching())
		{
			Agent->AIEndCrouch();
		}
	}
}

