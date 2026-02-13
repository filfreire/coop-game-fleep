// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacterManagerComponent.h"

USCharacterManagerComponent::USCharacterManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USCharacterManagerComponent::PostInitProperties()
{
	MaxAgentNum = 256; // Set maximum number of agents this manager can handle
	Super::PostInitProperties();
} 