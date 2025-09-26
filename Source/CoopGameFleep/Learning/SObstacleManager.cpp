// Fill out your copyright notice in the Description page of Project Settings.

#include "Learning/SObstacleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

USObstacleManager::USObstacleManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USObstacleManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize obstacles based on mode
	if (ObstacleMode == EObstacleMode::Static)
	{
		InitializeObstacles();
	}
}

void USObstacleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USObstacleManager::InitializeObstacles()
{
	// Clear existing obstacles
	ClearObstacles();

	// Generate obstacles
	for (int32 i = 0; i < MaxObstacles; i++)
	{
		FVector ObstaclePosition = GenerateRandomObstaclePosition(FVector::ZeroVector, 0.0f);
		ASObstacleActor* NewObstacle = CreateObstacleAtPosition(ObstaclePosition);
		if (NewObstacle)
		{
			CurrentObstacles.Add(NewObstacle);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("SObstacleManager: Initialized %d obstacles in %s mode"), 
		CurrentObstacles.Num(), 
		ObstacleMode == EObstacleMode::Static ? TEXT("Static") : TEXT("Dynamic"));
}

void USObstacleManager::ClearObstacles()
{
	for (ASObstacleActor* Obstacle : CurrentObstacles)
	{
		if (IsValid(Obstacle))
		{
			Obstacle->Destroy();
		}
	}
	CurrentObstacles.Empty();
}

void USObstacleManager::RegenerateObstacles()
{
	if (ObstacleMode == EObstacleMode::Dynamic)
	{
		ClearObstacles();
		InitializeObstacles();
		UE_LOG(LogTemp, Log, TEXT("SObstacleManager: Regenerated obstacles in dynamic mode"));
	}
}

bool USObstacleManager::IsLocationBlocked(const FVector& Location, float AgentRadius) const
{
	for (ASObstacleActor* Obstacle : CurrentObstacles)
	{
		if (IsValid(Obstacle) && Obstacle->IsLocationBlocked(Location, AgentRadius))
		{
			return true;
		}
	}
	return false;
}

void USObstacleManager::SetObstacleMode(EObstacleMode NewMode)
{
	ObstacleMode = NewMode;
	
	// If switching to static mode, initialize obstacles
	if (ObstacleMode == EObstacleMode::Static)
	{
		InitializeObstacles();
	}
	// If switching to dynamic mode, clear obstacles (they'll be generated on demand)
	else if (ObstacleMode == EObstacleMode::Dynamic)
	{
		ClearObstacles();
	}
}

FVector USObstacleManager::GenerateRandomObstaclePosition(const FVector& AvoidLocation, float AvoidRadius) const
{
	FVector Position;
	int32 Attempts = 0;
	const int32 MaxAttempts = 100;

	do
	{
		Position.X = EnvironmentCenter.X + FMath::RandRange(-EnvironmentBounds.X, EnvironmentBounds.X);
		Position.Y = EnvironmentCenter.Y + FMath::RandRange(-EnvironmentBounds.Y, EnvironmentBounds.Y);
		Position.Z = EnvironmentCenter.Z; // Place on ground level
		
		Attempts++;
	} while (!IsValidObstaclePosition(Position, AvoidLocation, AvoidRadius) && Attempts < MaxAttempts);

	return Position;
}

bool USObstacleManager::IsValidObstaclePosition(const FVector& Position, const FVector& AvoidLocation, float AvoidRadius) const
{
	// Check if position is within environment bounds
	if (Position.X < EnvironmentCenter.X - EnvironmentBounds.X || Position.X > EnvironmentCenter.X + EnvironmentBounds.X ||
		Position.Y < EnvironmentCenter.Y - EnvironmentBounds.Y || Position.Y > EnvironmentCenter.Y + EnvironmentBounds.Y)
	{
		return false;
	}

	// Check distance from avoid location
	if (AvoidRadius > 0.0f && FVector::Dist(Position, AvoidLocation) < AvoidRadius)
	{
		return false;
	}

	// Check distance from existing obstacles
	for (ASObstacleActor* Obstacle : CurrentObstacles)
	{
		if (IsValid(Obstacle))
		{
			float Distance = FVector::Dist(Position, Obstacle->GetActorLocation());
			if (Distance < MinObstacleSize) // Minimum distance between obstacles
			{
				return false;
			}
		}
	}

	return true;
}

ASObstacleActor* USObstacleManager::CreateObstacleAtPosition(const FVector& Position)
{
	if (!GetWorld() || !ObstacleClass)
	{
		return nullptr;
	}

	// Spawn obstacle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ASObstacleActor* NewObstacle = GetWorld()->SpawnActor<ASObstacleActor>(ObstacleClass, Position, FRotator::ZeroRotator, SpawnParams);
	
	if (NewObstacle)
	{
		// Set random size
		float Size = FMath::RandRange(MinObstacleSize, MaxObstacleSize);
		NewObstacle->InitializeObstacle(Size, Size * 1.5f, Size); // Height is 1.5x width/depth
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("SObstacleManager: Created obstacle at %s with size %f"), 
			*Position.ToString(), Size);
	}

	return NewObstacle;
}

