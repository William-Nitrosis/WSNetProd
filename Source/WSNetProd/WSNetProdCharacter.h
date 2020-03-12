// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UserWidget.h"
#include "Blueprint/UserWidget.h"
#include "WSNetProdCharacter.generated.h"

UCLASS(config=Game)
class AWSNetProdCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AWSNetProdCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
		FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** First person mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* FirstPersonMesh;

	/** First person gun actor - slot 1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UChildActorComponent* FirstPersonGunActorSlot1;

	/** First person gun actor - slot 1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UChildActorComponent* FirstPersonGunActorSlot2;

	/** Third person gun mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* ThirdPersonGunMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UChildActorComponent* CurrentlyEquipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxHead;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxTorso;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxLeftArmUpper;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxLeftArmLower;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxRightArmUpper;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxRightArmLower;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxLeftLeg;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UBoxComponent* CBoxRightLeg;
	

	/** Firing variable */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bFiring;

	/* Crosshair variables */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		int CrosshairArrayMaxIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		int CrosshairCurrentActiveArrayIndex;

//	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
//		TArray<UUserWidget*> CrosshairArray;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
		void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFiring();

	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFiring();

	UFUNCTION(Server, Reliable)
		void HandleSlotInput();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

