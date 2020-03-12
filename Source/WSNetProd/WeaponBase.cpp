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
	FHitResult SingleHit;
	FVector BulletStart = PlayerCharacter->GetFollowCamera()->GetComponentLocation();
	FVector BulletEnd = PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance);
	FCollisionObjectQueryParams Objects;
	FCollisionQueryParams Params;
	
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(PlayerCharacter);

	//GetWorld()->LineTraceMultiByObjectType(MultiHit, BulletStart, BulletEnd, Objects, Params);
	
	bool BulletTrace = GetWorld()->LineTraceSingleByObjectType(SingleHit, BulletStart, BulletEnd, Objects, Params);
	

	DrawDebugLine(GetWorld(), PlayerCharacter->GetFollowCamera()->GetComponentLocation(), (PlayerCharacter->GetFollowCamera()->GetComponentLocation() + (PlayerCharacter->GetFollowCamera()->GetForwardVector() * BulletDistance)), FColor::Green, false, 10, 0, 5);

	if (BulletTrace && IsValid(Cast<AWSNetProdCharacter>(SingleHit.Actor))) // Has the trace hit anything & Is the actor a player?
	{
		Params.AddIgnoredComponent(Cast<AWSNetProdCharacter>(SingleHit.Actor)->GetCapsuleComponent()); // Ignore the player capsule component so we can hit the hitboxs 
		//bool SecondBulletTrace = GetWorld()->LineTraceSingleByObjectType(SingleHit, BulletStart, BulletEnd, Objects, Params); // Recast the same line
		bool SecondBulletTrace = GetWorld()->LineTraceSingleByChannel(SingleHit, BulletStart, BulletEnd, ECC_Visibility, Params);
		
		if (SecondBulletTrace && IsValid(Cast<AWSNetProdCharacter>(SingleHit.GetComponent()->GetAttachmentRootActor()))) // has the trace hit anything & if there is a component, is it attached to the player?
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *SingleHit.Component->GetName());
		}
	}
	
	/*for (auto Hit : MultiHit)
	{
		if (IsValid(Cast<AWSNetProdCharacter>(Hit.Actor))) // is it a player character?
		{
			Params.AddIgnoredComponent(Cast<AWSNetProdCharacter>(Hit.Actor)->GetCapsuleComponent());
			GetWorld()->LineTraceMultiByObjectType(MultiHit, BulletStart, BulletEnd, Objects, Params);

			for (auto Hit : MultiHit)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s"), *Hit.Component->GetName());
				break;
			}
			
			Params.ClearIgnoredComponents();
		}
	}*/
	
	//MultiHit.Empty();
	
}
