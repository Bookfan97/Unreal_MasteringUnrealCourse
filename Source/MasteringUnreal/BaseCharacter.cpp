// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Spring Arm Setup
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bAbsoluteRotation = true;
	SpringArm->TargetArmLength = 1000.f;
	SpringArm->SetupAttachment(RootComponent);

	//Camera Setup
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->FieldOfView = 110.f;
	Camera->SetupAttachment(SpringArm);

	//Setup Forward Arrow Component
	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Forward Direction"));
	ForwardDirection->SetupAttachment(RootComponent);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);

	//Facing the correction direction
	bIsFiring = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//Allows character to walk up stairs
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxStepHeight = 45.f;
	ExplosionEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Explosion Effect"));
	ExplosionEffect->bAutoActivate = false;
	ExplosionEffect->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);
	JumpMaxHoldTime = 0.25f;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->JumpZVelocity = 800.f;
	GetCharacterMovement()->GravityScale = 2.0f;
	JumpMaxCount = 2;
	bIsSprinting = false;
	BaseRunSpeed = 600.f;
	MaxSprint = 2.0f;
	CurrentSprint = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpringArm->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, -25, 0)));
	Camera->Activate();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float rampThisFrame = (DeltaTime / TimeToMaxSpeed) * MaxSprint;
	if (bIsSprinting)
	{
		CurrentSprint += rampThisFrame;
	}
	else
	{
		CurrentSprint -= rampThisFrame;
	}
	CurrentSprint = FMath::Clamp(CurrentSprint, 1.f, MaxSprint);
	GetCharacterMovement()->MaxWalkSpeed = BaseRunSpeed * CurrentSprint;
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind action mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ABaseCharacter::FireStart);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ABaseCharacter::SprintStart);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ABaseCharacter::SprintEnd);

	//Bind Axis mappings
	InputComponent->BindAxis("ChangeCameraHeight", this, &ABaseCharacter::ChangeCameraHeight);
	InputComponent->BindAxis("RotateCamera", this, &ABaseCharacter::RotateCamera);
	InputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);
}

void ABaseCharacter::MoveForward(float amount)
{
	if (Controller && amount)
	{
		AddMovementInput(SpringArm->GetForwardVector(), amount);
	}
}

void ABaseCharacter::MoveRight(float amount)
{
	if (Controller && amount)
	{
		AddMovementInput(SpringArm->GetRightVector(), amount);
	}
}

void ABaseCharacter::RotateCamera(float amount)
{
	if (Controller && amount)
	{
		FVector rot = SpringArm->GetComponentRotation().Euler();
		rot += FVector(0, 0, amount);
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::ChangeCameraHeight(float amount)
{
	if (Controller && amount)
	{
		FVector rot = SpringArm->GetComponentRotation().Euler();
		float newHeight = rot.Y;
		newHeight += amount;
		newHeight = FMath::Clamp(newHeight, -45.f, -5.f);
		rot = FVector(0, newHeight, rot.Z);
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::SprintStart()
{
	bIsSprinting = true;
}

void ABaseCharacter::SprintEnd()
{
	bIsSprinting = false;
}

void ABaseCharacter::FireStart()
{
	if (!bIsFiring)
	{
		bIsFiring = true;
	}
}

void ABaseCharacter::Fired()
{
	if (!bIsFiring) return;
	bIsFiring = false;
	FVector HandLocation = GetMesh()->GetBoneLocation(FName("LeftHandMiddle1"));
	if (ExplosionEffect)
	{
		ExplosionEffect->SetWorldLocation(HandLocation);
		ExplosionEffect->Activate(true);
	}
	float distance = 10000;
	FVector direction = ForwardDirection->GetForwardVector();
	FVector start = HandLocation;
	FVector end = start + (direction * distance);
	FHitResult outHit;
	bool HasHitSomething = GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECollisionChannel::ECC_Destructible);
	FColor color = FColor::MakeRandomColor();
	if (HasHitSomething)
	{
		DrawDebugLine(GetWorld(), start, outHit.ImpactPoint, color, true, 1.f, 0, 12.333);
		if (!outHit.GetActor()->IsRootComponentMovable()) return;
		TArray<UStaticMeshComponent*> Components;
		outHit.GetActor()->GetComponents<UStaticMeshComponent>(Components);
		for (auto& mesh : Components)
		{
			mesh->AddForce(direction * 100000000.f);
		}
	}
	else
	{
		DrawDebugLine(GetWorld(), start, end, color, true, 1.f, 0, 12.333);
	}
}