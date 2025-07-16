// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

UCLASS()
class COOPGAMEFLEEP_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	/* Default FOV set on beg*/
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	void BeginZoom();

	void EndZoom();

	ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	void StartFire();

	void StopFire();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	int RifleAmmo;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/* Pawn Died previously */
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

public:
	// Learning Agents support - make bDied accessible
	UFUNCTION(BlueprintCallable, Category = "Learning")
	bool IsDead() const { return bDied; }

protected:

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	bool UpdatePlayerRifleAmmoCount(int ammount);

	UFUNCTION(BlueprintCallable, Category = "Player")
	int CurrentPlayerRifleAmmoCount();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void ResetCharacterPosition();

	// Learning Agents support
	UFUNCTION(BlueprintCallable, Category = "Learning")
	void ResetForLearning(FVector NewLocation, FRotator NewRotation = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "Learning")
	bool IsAvailableForLearning() const;

	// Flag to control whether player input is enabled (useful during learning)
	UPROPERTY(BlueprintReadWrite, Category = "Learning")
	bool bPlayerInputEnabled = true;

};
