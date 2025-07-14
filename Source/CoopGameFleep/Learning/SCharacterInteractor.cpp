// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterInteractor.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsActions.h"
#include "LearningAgentsManager.h"
#include "STargetActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SCharacter.h"

USCharacterInteractor::USCharacterInteractor()
{
	TargetActor = nullptr;
}

void USCharacterInteractor::SpecifyAgentObservation_Implementation(
	FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
	ULearningAgentsObservationSchema* InObservationSchema)
{
	// Define observations for the character learning task
	TMap<FName, FLearningAgentsObservationSchemaElement> CharacterObservations;

	// Character position relative to world
	CharacterObservations.Add("CharacterLocation", 
		ULearningAgentsObservations::SpecifyLocationObservation(
			InObservationSchema, MaxObservationDistance, "CharacterLocationObservation"));

	// Character velocity
	CharacterObservations.Add("CharacterVelocity", 
		ULearningAgentsObservations::SpecifyVelocityObservation(InObservationSchema, MaxVelocity));

	// Character forward direction
	CharacterObservations.Add("CharacterDirection", 
		ULearningAgentsObservations::SpecifyDirectionObservation(InObservationSchema, "CharacterDirectionObservation"));

	// Target position relative to world
	CharacterObservations.Add("TargetLocation", 
		ULearningAgentsObservations::SpecifyLocationObservation(
			InObservationSchema, MaxObservationDistance, "TargetLocationObservation"));

	// Direction from character to target
	CharacterObservations.Add("DirectionToTarget", 
		ULearningAgentsObservations::SpecifyDirectionObservation(InObservationSchema, "DirectionToTargetObservation"));

	// Distance to target (normalized)
	CharacterObservations.Add("DistanceToTarget", 
		ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema, MaxObservationDistance));

	// Set the complete observation schema
	OutObservationSchemaElement = ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, CharacterObservations);
}

void USCharacterInteractor::GatherAgentObservation_Implementation(
	FLearningAgentsObservationObjectElement& OutObservationObjectElement,
	ULearningAgentsObservationObject* InObservationObject, const int32 AgentId)
{
	// Get the character agent
	const ASCharacter* Character = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (!Character || !TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get character or target for agent %d"), AgentId);
		return;
	}

	// Gather character observations
	TMap<FName, FLearningAgentsObservationObjectElement> CharacterObservationObject;

	// Character location
	CharacterObservationObject.Add("CharacterLocation", 
		ULearningAgentsObservations::MakeLocationObservation(InObservationObject, Character->GetActorLocation()));

	// Character velocity
	FVector CharacterVelocity = FVector::ZeroVector;
	if (const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
	{
		CharacterVelocity = MovementComp->Velocity;
	}
	CharacterObservationObject.Add("CharacterVelocity", 
		ULearningAgentsObservations::MakeVelocityObservation(InObservationObject, CharacterVelocity));

	// Character forward direction
	CharacterObservationObject.Add("CharacterDirection", 
		ULearningAgentsObservations::MakeDirectionObservation(InObservationObject, Character->GetActorForwardVector()));

	// Target location
	CharacterObservationObject.Add("TargetLocation", 
		ULearningAgentsObservations::MakeLocationObservation(InObservationObject, TargetActor->GetActorLocation()));

	// Direction from character to target
	FVector DirectionToTarget = (TargetActor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
	CharacterObservationObject.Add("DirectionToTarget", 
		ULearningAgentsObservations::MakeDirectionObservation(InObservationObject, DirectionToTarget));

	// Distance to target
	float DistanceToTarget = FVector::Dist(Character->GetActorLocation(), TargetActor->GetActorLocation());
	CharacterObservationObject.Add("DistanceToTarget", 
		ULearningAgentsObservations::MakeFloatObservation(InObservationObject, DistanceToTarget));

	// Set the complete observation object
	OutObservationObjectElement = ULearningAgentsObservations::MakeStructObservation(InObservationObject, CharacterObservationObject);
}

void USCharacterInteractor::SpecifyAgentAction_Implementation(
	FLearningAgentsActionSchemaElement& OutActionSchemaElement,
	ULearningAgentsActionSchema* InActionSchema)
{
	// Define actions for character movement
	TMap<FName, FLearningAgentsActionSchemaElement> CharacterActions;

	// Movement input (forward/backward)
	CharacterActions.Add("MoveForward", 
		ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "MoveForwardAction"));

	// Movement input (left/right)
	CharacterActions.Add("MoveRight", 
		ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "MoveRightAction"));

	// Rotation input (yaw)
	CharacterActions.Add("Turn", 
		ULearningAgentsActions::SpecifyFloatAction(InActionSchema, 1.0f, "TurnAction"));

	// Set the complete action schema
	OutActionSchemaElement = ULearningAgentsActions::SpecifyStructAction(InActionSchema, CharacterActions);
}

void USCharacterInteractor::PerformAgentAction_Implementation(
	const ULearningAgentsActionObject* InActionObject,
	const FLearningAgentsActionObjectElement& InActionObjectElement,
	const int32 AgentId)
{
	// Get the character agent
	ASCharacter* Character = Cast<ASCharacter>(Manager->GetAgent(AgentId, ASCharacter::StaticClass()));
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get character for agent %d"), AgentId);
		return;
	}

	// Extract actions from the action object
	TMap<FName, FLearningAgentsActionObjectElement> CharacterActionObjects;
	if (!ULearningAgentsActions::GetStructAction(CharacterActionObjects, InActionObject, InActionObjectElement))
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get struct action for agent %d"), AgentId);
		return;
	}

	// Get movement actions
	float MoveForwardValue = 0.0f;
	float MoveRightValue = 0.0f;
	float TurnValue = 0.0f;

	const FLearningAgentsActionObjectElement* MoveForwardAction = CharacterActionObjects.Find("MoveForward");
	if (MoveForwardAction && !ULearningAgentsActions::GetFloatAction(MoveForwardValue, InActionObject, *MoveForwardAction))
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get MoveForward action for agent %d"), AgentId);
	}

	const FLearningAgentsActionObjectElement* MoveRightAction = CharacterActionObjects.Find("MoveRight");
	if (MoveRightAction && !ULearningAgentsActions::GetFloatAction(MoveRightValue, InActionObject, *MoveRightAction))
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get MoveRight action for agent %d"), AgentId);
	}

	const FLearningAgentsActionObjectElement* TurnAction = CharacterActionObjects.Find("Turn");
	if (TurnAction && !ULearningAgentsActions::GetFloatAction(TurnValue, InActionObject, *TurnAction))
	{
		UE_LOG(LogTemp, Error, TEXT("SCharacterInteractor: Failed to get Turn action for agent %d"), AgentId);
	}

	// Apply movement inputs to the character
	if (FMath::Abs(MoveForwardValue) > 0.01f)
	{
		Character->AddMovementInput(Character->GetActorForwardVector(), MoveForwardValue);
	}

	if (FMath::Abs(MoveRightValue) > 0.01f)
	{
		Character->AddMovementInput(Character->GetActorRightVector(), MoveRightValue);
	}

	// Apply rotation input
	if (FMath::Abs(TurnValue) > 0.01f)
	{
		Character->AddControllerYawInput(TurnValue);
	}
} 