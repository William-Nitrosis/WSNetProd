// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine.h"
#include "TimerManager.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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
	FVector BulletStart = PlayerCharacter->GetFollowCamera()->GetComponentLocation();
	FVector BulletEnd = PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance);
	FCollisionObjectQueryParams Objects;
	FCollisionQueryParams Params;
	
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PlayerCharacter);
	/*GetWorld()->LineTraceMultiByChannel(
		MultiHit,
		PlayerCharacter->GetFollowCamera()->GetComponentLocation(),
		(PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance)),
		ECC_WorldDynamic,
		Params);*/

	GetWorld()->LineTraceMultiByObjectType(MultiHit, BulletStart, BulletEnd, Objects, Params);

	DrawDebugLine(GetWorld(), PlayerCharacter->GetFollowCamera()->GetComponentLocation(), (PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance)), FColor::Green, false, 10, 0, 5);

	for (auto Hit : MultiHit)
	{
		//"MyCharacter's Name is %s"
		UE_LOG(LogTemp, Warning, TEXT("ACTOR HIT %s"), *Hit.Actor->GetName());
		
		if (IsValid(Cast<AWSNetProdCharacter>(Hit.Actor)))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("player hit"));
			
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Hit.Component->GetName());

			// Create a damage event  
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);
			Cast<AWSNetProdCharacter>(Hit.Actor)->TakeDamage(Damage, DamageEvent, GetWorld()->GetFirstPlayerController(), this);
	
		}
	}

	MultiHit.Empty();
	
}
