// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsManager.h"
#include "SCharacterManagerComponent.generated.h"

/**
 * Manager component for SCharacter learning agents
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (LearningAgents), meta = (BlueprintSpawnableComponent))
class COOPGAMEFLEEP_API USCharacterManagerComponent : public ULearningAgentsManager
{
	GENERATED_BODY()

public:
	USCharacterManagerComponent();

protected:
	virtual void PostInitProperties() override;
}; 