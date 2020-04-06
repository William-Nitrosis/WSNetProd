// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WSNetProdCharacter.h"
#include "WeaponBase.generated.h"

UCLASS()
class WSNETPROD_API AWeaponBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USceneComponent* SceneRoot;

	/** Gun mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* GunMesh;

	/** Barrel particle emitter */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USceneComponent* BarrelParticleEmitterLocation;

	UFUNCTION(BlueprintCallable)
		void HandleInput();

	UFUNCTION()
		FORCEINLINE int GetMagazineSize() const { return MagazineSize; }

	UFUNCTION()
		FORCEINLINE int GetDamage() const { return Damage; }



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Damage */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		float Damage = 1.0f;

	/** Fire rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		float FireRate = 0.25f;

	FTimerHandle FiringTimer;

	/** Magazine size */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		int MagazineSize = 30;

	/** Total ammo */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		int TotalAmmo = 90;

	/** Recoil strength */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		float RecoilStrength = 1.0f;

	/** Bullet distance */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		float BulletDistance = 5000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gun stats")
		int CurrentAmmo;

	UFUNCTION(BlueprintCallable)
		void StopFire();

	UFUNCTION(BlueprintCallable)
		void FireBullet();

	bool bCanFireGun = true;

	AWSNetProdCharacter* PlayerCharacter;




public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
