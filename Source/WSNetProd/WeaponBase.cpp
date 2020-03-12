// Fill out your copyright notice in the Description page of Project Settings.

#include "Engine.h"
#include "TimerManager.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Classes/Engine/World.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "WeaponBase.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetupAttachment(RootComponent);

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(SceneRoot);

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
	FHitResult SingleHit;
	FVector BulletStart = PlayerCharacter->GetFollowCamera()->GetComponentLocation();
	FVector BulletEnd = PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance);
	FCollisionObjectQueryParams Objects;
	FCollisionQueryParams Params;
	
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PlayerCharacter);
	
	bool BulletTrace = GetWorld()->LineTraceSingleByObjectType(SingleHit, BulletStart, BulletEnd, Objects, Params);
	

	DrawDebugLine(GetWorld(), PlayerCharacter->GetFollowCamera()->GetComponentLocation(), (PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance)), FColor::Green, false, 10, 0, 5);

	if (BulletTrace && IsValid(Cast<AWSNetProdCharacter>(SingleHit.Actor))) // Has the trace hit anything & Is the actor a player?
	{
		Params.AddIgnoredComponent(Cast<AWSNetProdCharacter>(SingleHit.Actor)->GetCapsuleComponent()); // Ignore the player capsule component so we can hit the hitboxs 
		bool SecondBulletTrace = GetWorld()->LineTraceSingleByChannel(SingleHit, BulletStart, BulletEnd, ECC_Visibility, Params);
		
		if (SecondBulletTrace && IsValid(Cast<AWSNetProdCharacter>(SingleHit.GetComponent()->GetAttachmentRootActor()))) // has the trace hit anything & if there is a component, is it attached to the player?
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *SingleHit.Component->GetName());
		
			FVector NormalImpulse;
			FHitResult Hit;
			//UGameplayStatics::ApplyPointDamage(SingleHit.Actor.Get(), Damage, NormalImpulse, Hit, UGameplayStatics::GetPlayerControllerFromID(GetWorld(), 0), this, UDamageType::StaticClass());

			AWSNetProdCharacter* actor = Cast<AWSNetProdCharacter>(SingleHit.Actor.Get());
			this->PlayerCharacter->DealDamage_Custom(1.0f, SingleHit.Actor.Get());
			//actor->DealDamage(1.0f, )
		}
	}
	
	
}
