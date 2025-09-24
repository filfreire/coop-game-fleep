// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SObstacleActor.h"
#include "Learning/ObstacleTypes.h"
#include "SObstacleManager.generated.h"



/**
 * Manages obstacles in the training environment
 * Supports both static and dynamic modes
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class COOPGAMEFLEEP_API USObstacleManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	USObstacleManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Obstacle mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Management")
	EObstacleMode ObstacleMode = EObstacleMode::Static;

	// Obstacle configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	int32 MaxObstacles = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinObstacleSize = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MaxObstacleSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinDistanceFromAgents = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinDistanceFromTarget = 200.0f;

	// Environment bounds for obstacle placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FVector EnvironmentCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	FVector EnvironmentBounds = FVector(2000.0f, 2000.0f, 0.0f);

	// Obstacle class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	TSubclassOf<ASObstacleActor> ObstacleClass = ASObstacleActor::StaticClass();

	// Current obstacles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle Management")
	TArray<ASObstacleActor*> CurrentObstacles;

	// Initialize obstacles for the environment
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void InitializeObstacles();

	// Clear all obstacles
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void ClearObstacles();

	// Regenerate obstacles (for dynamic mode)
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void RegenerateObstacles();

	// Check if a location is blocked by any obstacle
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	bool IsLocationBlocked(const FVector& Location, float AgentRadius = 50.0f) const;

	// Get all obstacles
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	TArray<ASObstacleActor*> GetObstacles() const { return CurrentObstacles; }

	// Set obstacle mode
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void SetObstacleMode(EObstacleMode NewMode);

private:
	// Generate a random position for an obstacle
	FVector GenerateRandomObstaclePosition(const FVector& AvoidLocation, float AvoidRadius) const;

	// Check if a position is valid for obstacle placement
	bool IsValidObstaclePosition(const FVector& Position, const FVector& AvoidLocation, float AvoidRadius) const;

	// Create a single obstacle at the given position
	ASObstacleActor* CreateObstacleAtPosition(const FVector& Position);
};

