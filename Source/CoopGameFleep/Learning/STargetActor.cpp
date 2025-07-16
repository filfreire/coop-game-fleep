// Fill out your copyright notice in the Description page of Project Settings.

#include "STargetActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ASTargetActor::ASTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and setup mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set a default sphere mesh if available
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMeshAsset.Object);
		MeshComponent->SetWorldScale3D(FVector(1.5f)); // Make it a bit larger
	}

	// Set collision to block all
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void ASTargetActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASTargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASTargetActor::ResetToRandomLocation(FVector Center, FVector Bounds)
{
	// Generate random location within bounds
	FVector RandomLocation;
	RandomLocation.X = Center.X + FMath::RandRange(-Bounds.X, Bounds.X);
	RandomLocation.Y = Center.Y + FMath::RandRange(-Bounds.Y, Bounds.Y);
	RandomLocation.Z = Center.Z + FMath::RandRange(-Bounds.Z, Bounds.Z);

	SetActorLocation(RandomLocation);
}

bool ASTargetActor::IsLocationWithinReach(FVector Location, float Distance) const
{
	float ActualDistance = FVector::Dist(GetActorLocation(), Location);
	return ActualDistance <= Distance;
} 