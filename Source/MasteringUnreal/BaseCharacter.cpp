// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"




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
	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Fordward Direction"));
	ForwardDirection->SetupAttachment(RootComponent);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Destructible, ECollisionResponse::ECR_Ignore);
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
}