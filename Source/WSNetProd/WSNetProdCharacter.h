// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Character.h"
#include "WSNetProdCharacter.generated.h"


class UUserWidget;

USTRUCT(BlueprintType)
struct FCrosshair
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		TSubclassOf<UUserWidget> WidgetFile;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		UUserWidget* CrosshairsObjectWidgetArray;

};

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

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure)
		FORCEINLINE int GetCurrentAmmo() const { return CurrentAmmo; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
		void SetCurrentHealth(float healthValue);

	UFUNCTION(BlueprintCallable)
		void SetCurrentAmmo(float AmmoValue);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		TArray<FCrosshair> CrosshairArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		int CrosshairArrayMaxIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		int CrosshairCurrentActiveArrayIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		UUserWidget* CrosshairActiveCrosshair;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair")
		bool bCrosshairVisable = true;

	/* Crosshair functions */

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
		bool GetIsMoving();

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
		bool GetIsFiring();

	


	UFUNCTION(Server, Reliable)
		void ServerApplyDamage(float someDEEPS, AActor* target);

	UFUNCTION(Server, Reliable)
		void ServerLineTrace(FVector LineTraceStart, FVector LineTraceEnd, AActor* SourceGun, AActor* SourcePlayer);

	

	

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
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CurrentHealth)
		float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
		void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	UPROPERTY(Replicated)
		int CurrentAmmo;

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFiring();

	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StopFiring();

	UFUNCTION(Server, Reliable)
		void HandleSlotInput();

	UFUNCTION(Server, Reliable)
		void ReloadGun(AActor* ReloadTargetPlayer);



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

