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
	SpringArm->TargetArmLength = 1000.0f;
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
	GetCharacterMovement()->SetWalkableFloorAngle(50.0f);
	GetCharacterMovement()->MaxStepHeight = 45.f;
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
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind action mappings
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ABaseCharacter::Fired);

	//Bind Axis mappings
	InputComponent->BindAxis("ChangeCameraHeight", this, &ABaseCharacter::ChangeCameraHeight);
	InputComponent->BindAxis("RotateCamera", this, &ABaseCharacter::RotateCamera);
	InputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);
}

void ABaseCharacter::MoveForward(float amount)
{
	if(Controller && amount)
	{
		AddMovementInput(SpringArm->GetForwardVector(), amount);
	}
}

void ABaseCharacter::MoveRight(float amount)
{
	if(Controller && amount)
	{
		AddMovementInput(SpringArm->GetRightVector(), amount);
	}
}

void ABaseCharacter::RotateCamera(float amount)
{
	if(Controller && amount)
	{
		FVector rot = SpringArm->GetComponentRotation().Euler();
		rot += FVector(0,0,amount);
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::ChangeCameraHeight(float amount)
{
	if(Controller && amount)
	{
		FVector rot = SpringArm->GetComponentRotation().Euler();
		float newHeight = rot.Y;
		newHeight += amount;
		newHeight = FMath::Clamp(newHeight, -45.f, -5.f);
		rot= FVector(0, newHeight, rot.Z);
		SpringArm->SetWorldRotation(FQuat::MakeFromEuler(rot));
	}
}

void ABaseCharacter::FireStart()
{
	float distance = 10000;
	FVector direction = ForwardDirection->GetForwardVector();
	FVector start = GetActorLocation() + (direction *  60.f);
	FVector end = start + (direction * distance);
	FHitResult outHit;
	bool HasHitSomething = GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECollisionChannel::ECC_Destructible);
	FColor color = FColor::MakeRandomColor();
	if(HasHitSomething)
	{
		DrawDebugLine(GetWorld(), start, outHit.ImpactPoint, color, true, 1.f, 0, 12.333);
		if(!outHit.GetActor()->IsRootComponentMovable()) return;
		TArray<UStaticMeshComponent*> Components;
		outHit.GetActor()->GetComponents<UStaticMeshComponent>(Components);
		for(auto &mesh : Components)
		{
			mesh->AddForce(direction * 10000000);
		}
	}
	else
	{
		DrawDebugLine(GetWorld(), start, end, color, true, 1.f, 0, 12.333);
	}
}

void ABaseCharacter::Fired()
{
}
