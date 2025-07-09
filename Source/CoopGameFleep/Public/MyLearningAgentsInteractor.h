// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "MyLearningAgentsInteractor.generated.h"

class ASCharacter;

/**
 * Learning Agents Interactor for character movement and navigation
 */
UCLASS()
class COOPGAMEFLEEP_API UMyLearningAgentsInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()

public:
	UMyLearningAgentsInteractor();

	// Observation system
	virtual void SpecifyAgentObservation_Implementation(
		FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
		ULearningAgentsObservationSchema* InObservationSchema) override;

	virtual void GatherAgentObservation_Implementation(
		FLearningAgentsObservationObjectElement& OutObservationObjectElement,
		ULearningAgentsObservationObject* InObservationObject, 
		const int32 AgentId) override;

	// Action system
	virtual void SpecifyAgentAction_Implementation(
		FLearningAgentsActionSchemaElement& OutActionSchemaElement, 
		ULearningAgentsActionSchema* InActionSchema) override;

	virtual void PerformAgentAction_Implementation(
		const ULearningAgentsActionObject* InActionObject,
		const FLearningAgentsActionObjectElement& InActionObjectElement, 
		const int32 AgentId) override;

	// Configuration properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
	TSoftObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
	int32 OtherAgentCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
	float ObservationDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actions")
	float MaxMovementSpeed;
};
