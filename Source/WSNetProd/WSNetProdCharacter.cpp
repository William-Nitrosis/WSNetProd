// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "WSNetProdCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

//////////////////////////////////////////////////////////////////////////
// AWSNetProdCharacter

AWSNetProdCharacter::AWSNetProdCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
//	bUseControllerRotationPitch = false;
//	bUseControllerRotationYaw = false;
//	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	CameraBoom->TargetArmLength = 0.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the mesh to the end of the boom and let the boom adjust to match the controller orientation

	FirstPersonGunActorSlot1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("FirstPersonGunActorSlot1"));
	FirstPersonGunActorSlot1->SetupAttachment(FirstPersonMesh);

	FirstPersonGunActorSlot2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("FirstPersonGunActorSlot2"));
	FirstPersonGunActorSlot2->SetupAttachment(FirstPersonMesh);

	ThirdPersonGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonGunMesh"));
	ThirdPersonGunMesh->SetupAttachment(GetMesh());

	CBoxHead = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxHead"));
	CBoxHead->SetupAttachment(GetMesh());
	CBoxTorso = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxTorso"));
	CBoxTorso->SetupAttachment(GetMesh());
	CBoxLeftArmUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxLeftArmUpper"));
	CBoxLeftArmUpper->SetupAttachment(GetMesh());
	CBoxLeftArmLower = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxLeftArmLower"));
	CBoxLeftArmLower->SetupAttachment(GetMesh());
	CBoxRightArmUpper = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxRightArmUpper"));
	CBoxRightArmUpper->SetupAttachment(GetMesh());
	CBoxRightArmLower = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxRightArmLower"));
	CBoxRightArmLower->SetupAttachment(GetMesh());
	CBoxLeftLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxLeftLeg"));
	CBoxLeftLeg->SetupAttachment(GetMesh());
	CBoxRightLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("CBoxRightLeg"));
	CBoxRightLeg->SetupAttachment(GetMesh());

	// set mesh location/rotation in cap comp
	this->GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -95.0f));
	this->GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	
	CurrentlyEquipped = FirstPersonGunActorSlot1;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWSNetProdCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWSNetProdCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWSNetProdCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWSNetProdCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWSNetProdCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AWSNetProdCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AWSNetProdCharacter::TouchStopped);

	// Handle firing projectiles
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWSNetProdCharacter::StartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AWSNetProdCharacter::StopFiring);
}


void AWSNetProdCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Toggle visibility of skel mesh and arm on our self
	AWSNetProdCharacter* PlayerCharacter = Cast<AWSNetProdCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerCharacter == this) {
		this->GetMesh()->ToggleVisibility(false);
		ThirdPersonGunMesh->ToggleVisibility(false);
	} else
	{
		FirstPersonMesh->ToggleVisibility(false);
		FirstPersonGunActorSlot1->ToggleVisibility(false);
		FirstPersonGunActorSlot2->ToggleVisibility(false);
	}
}

// Called every frame
void AWSNetProdCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleSlotInput_Implementation();
	//UE_LOG(LogTemp, Warning, TEXT("%f"), GetCurrentHealth());
	
}

void AWSNetProdCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AWSNetProdCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AWSNetProdCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWSNetProdCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AWSNetProdCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AWSNetProdCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// Replicated Properties

void AWSNetProdCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AWSNetProdCharacter, CurrentHealth);
}

void AWSNetProdCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (Role == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
}

void AWSNetProdCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void AWSNetProdCharacter::SetCurrentHealth(float healthValue)
{
	if (Role == ROLE_Authority)
	{
		UE_LOG(LogTemp, Display, TEXT("%s %f"), *this->GetName() ,CurrentHealth)
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AWSNetProdCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return DamageTaken;
}

void AWSNetProdCharacter::StartFiring()
{
	bFiring = true;
}

void AWSNetProdCharacter::StopFiring()
{
	bFiring = false;
}

void AWSNetProdCharacter::HandleSlotInput_Implementation()
{
	AWeaponBase* CWeapon = Cast<AWeaponBase>(CurrentlyEquipped->GetChildActor());
	if (bFiring && CWeapon != nullptr)
	{
		CWeapon->HandleInput();
	}
}


void AWSNetProdCharacter::DealDamage_Custom_Implementation(float someDEEPS, AActor* target) {

	AWSNetProdCharacter* ptr = Cast<AWSNetProdCharacter>(target);
	if(ptr)
		ptr->SetCurrentHealth(CurrentHealth - someDEEPS);
}
