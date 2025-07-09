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

	// Learning Agents variables
	UPROPERTY(BlueprintReadOnly, Category = "Learning")
	int32 AgentID;

	UPROPERTY(BlueprintReadOnly, Category = "Learning")
	bool bFoundManager;

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

	// Learning Agents public interface
	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	void AIMoveForward(float Value) { MoveForward(Value); }

	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	void AIMoveRight(float Value) { MoveRight(Value); }

	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	void AIBeginCrouch() { BeginCrouch(); }

	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	void AIEndCrouch() { EndCrouch(); }

	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	bool IsDead() const { return bDied; }

	UFUNCTION(BlueprintCallable, Category = "Learning Agents")
	USHealthComponent* GetHealthComponent() const { return HealthComp; }

private:
	// Register with Learning Agents Manager
	void RegisterWithLearningAgentsManager();

};
