// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine.h"
#include "TimerManager.h"
#include "Engine/Classes/Engine/World.h"
#include "WeaponBase.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(RootComponent);

	BarrelParticleEmitterLocation = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelParticleEmitterLocation"));
	BarrelParticleEmitterLocation->SetupAttachment(GunMesh);

	bCanFireGun = true;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AWSNetProdCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AWeaponBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AWeaponBase::HandleInput()
{
	if (bCanFireGun)
	{
		bCanFireGun = false;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &AWeaponBase::StopFire, FireRate, false);
		FireBullet();
	}
}

void AWeaponBase::StopFire()
{
	bCanFireGun = true;
}

void AWeaponBase::FireBullet()
{
	TArray<FHitResult> MultiHit;
	FCollisionQueryParams Params;
	GetWorld()->LineTraceMultiByChannel(
		MultiHit,
		PlayerCharacter->GetFollowCamera()->GetComponentLocation(),
		(PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance)),
		ECC_Visibility,
		Params);

	/*

	
	for (auto hit : MultiHit)
	{
		PlayerCharacter = Cast<AWSNetProdCharacter>(hit.Actor);
		if (PlayerCharacter != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("player hit"));
		}
	}
	*/
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Shot fired"));
}
