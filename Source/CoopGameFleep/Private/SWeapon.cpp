// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"

#include "Chaos/ChaosEngineInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

#include <CoopGameFleep/CoopGameFleep.h>
#include <SCharacter.h>

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVarDebugWeapon(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing,
                                        TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;

	FireRate = 600;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / FireRate;
}

void ASWeapon::Fire()
{
	// trace the world from pawn eyes to crosshair location

	AActor* WeaponOwner = GetOwner();

	ASCharacter* WeaponOwnerCharacter = Cast<ASCharacter>(WeaponOwner);

	if (WeaponOwner && WeaponOwnerCharacter->CurrentPlayerRifleAmmoCount() > 0)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;

		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle "Target" parameter
		FVector TracerEndpoint = TraceEnd;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Hit! Process Damage
			AActor* HitActor = Hit.GetActor();
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float FinalDamage = BaseDamage;
			// if we hit vulnerable spot, multipy base damage by 4
			if (SurfaceType == SURFACE_FLESH_VULNERABLE)
			{
				FinalDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, FinalDamage, ShotDirection, Hit,
			                                   WeaponOwner->GetInstigatorController(), this, DamageType);

			UParticleSystem* SelectedEffect = nullptr;

			switch (SurfaceType)
			{
				case SURFACE_FLESH_DEFAULT:
				case SURFACE_FLESH_VULNERABLE:
					SelectedEffect = FleshImpactEffect;
					break;
				case SurfaceType_Default:
				default:
					SelectedEffect = DefaultImpactEffect;
					break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint,
				                                         Hit.ImpactNormal.Rotation(), true);
			}

			TracerEndpoint = Hit.ImpactPoint;
		}

		if (DebugWeaponDrawing != 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndpoint);

		LastFireTime = GetWorld()->TimeSeconds;
		WeaponOwnerCharacter->UpdatePlayerRifleAmmoCount(-1);

		UE_LOG(LogTemp, Log, TEXT("Ammo changed: %s"),
		       *FString::FromInt(WeaponOwnerCharacter->CurrentPlayerRifleAmmoCount()));
	}
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShoots, this, &ASWeapon::Fire, TimeBetweenShots, true,
	                                FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShoots);
}

void ASWeapon::PlayFireEffects(FVector TracerEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp =
		    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEnd);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner)
	{
		APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());

		if (PlayerController)
		{
			PlayerController->ClientStartCameraShake(FireCamShake);
		}
	}
}
