// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObstacleTypes.generated.h"

UENUM(BlueprintType)
enum class EObstacleMode : uint8
{
	Static UMETA(DisplayName = "Static Mode"),
	Dynamic UMETA(DisplayName = "Dynamic Mode")
};
