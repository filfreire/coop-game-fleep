// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "SCharacterInteractor.generated.h"

class ASTargetActor;

/**
 * Interactor for SCharacter learning agents
 */
UCLASS()
class COOPGAMEFLEEP_API USCharacterInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()

public:
	USCharacterInteractor();

	virtual void SpecifyAgentObservation_Implementation(
		FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
		ULearningAgentsObservationSchema* InObservationSchema) override;

	virtual void GatherAgentObservation_Implementation(
		FLearningAgentsObservationObjectElement& OutObservationObjectElement,
		ULearningAgentsObservationObject* InObservationObject,
		const int32 AgentId) override;
	
	virtual void SpecifyAgentAction_Implementation(
		FLearningAgentsActionSchemaElement& OutActionSchemaElement,
		ULearningAgentsActionSchema* InActionSchema) override;

	virtual void PerformAgentAction_Implementation(
		const ULearningAgentsActionObject* InActionObject,
		const FLearningAgentsActionObjectElement& InActionObjectElement,
		const int32 AgentId) override;

	// Reference to the target actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	ASTargetActor* TargetActor;

	// Observation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
	float MaxObservationDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
	float MaxVelocity = 1000.0f;
}; 