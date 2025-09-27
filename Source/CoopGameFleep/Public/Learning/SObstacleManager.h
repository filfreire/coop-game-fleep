// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SObstacleActor.h"
#include "Learning/ObstacleTypes.h"
#include "GameFramework/Volume.h"
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
	int32 MaxObstacles = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinObstacleSize = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MaxObstacleSize = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinDistanceFromAgents = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Configuration")
	float MinDistanceFromTarget = 200.0f;

	// Location volume for obstacle placement (if available)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	AVolume* LocationVolume = nullptr;

	// Fallback environment bounds for obstacle placement (used if no LocationVolume)
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

	// Initialize obstacles with smart placement around agents and targets
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void InitializeObstaclesWithSmartPlacement(const FVector& AgentLocation, const FVector& TargetLocation);

	// Set the location volume for obstacle placement
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void SetLocationVolume(AVolume* NewLocationVolume);

	// Find and set location volume automatically
	UFUNCTION(BlueprintCallable, Category = "Obstacle Management")
	void FindAndSetLocationVolume();

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

	// Find ground level at a given position using line trace
	float FindGroundLevel(const FVector& Position) const;
};

