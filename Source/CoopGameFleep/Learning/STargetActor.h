// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STargetActor.generated.h"

UCLASS()
class COOPGAMEFLEEP_API ASTargetActor : public AActor
{
	GENERATED_BODY()

  public:
	ASTargetActor();

  protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

  public:
	virtual void Tick(float DeltaTime) override;

	// Reset the target to a new random location within the specified bounds
	UFUNCTION(BlueprintCallable, Category = "Learning")
	void ResetToRandomLocation(FVector Center, FVector Bounds);

	// Check if the given location is within reach distance of this target
	UFUNCTION(BlueprintCallable, Category = "Learning")
	bool IsLocationWithinReach(FVector Location, float Distance = 150.0f) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
	float ReachDistance = 150.0f;
};
