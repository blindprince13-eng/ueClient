// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerFPSDemoCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MultiplayerFPSDemo.h"

AMultiplayerFPSDemoCharacter::AMultiplayerFPSDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// 启用网络同步
	bReplicates = true;
	// 初始化血量
	CurrentHealth = MaxHealth;
}

void AMultiplayerFPSDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMultiplayerFPSDemoCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMultiplayerFPSDemoCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiplayerFPSDemoCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiplayerFPSDemoCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMultiplayerFPSDemoCharacter::LookInput);
	}
	else
	{
		// 这里改用了 LogTemp 以防万一
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}


void AMultiplayerFPSDemoCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMultiplayerFPSDemoCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AMultiplayerFPSDemoCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMultiplayerFPSDemoCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMultiplayerFPSDemoCharacter::DoJumpStart()
{
	Jump();
}

void AMultiplayerFPSDemoCharacter::DoJumpEnd()
{
	StopJumping();
}

// 必须：告诉引擎哪些变量需要同步
void AMultiplayerFPSDemoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiplayerFPSDemoCharacter, CurrentHealth);
}

// 受伤逻辑：只有在服务器（HasAuthority）上才会执行
float AMultiplayerFPSDemoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. 必须先调用父类
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 2. 核心逻辑全部包在 Authority (服务器) 判定里
	if (HasAuthority())
	{
		if (CurrentHealth <= 0.0f) return 0.0f;

		// 计算实际伤害并扣除
		ActualDamage = FMath::Min(ActualDamage, CurrentHealth);
		CurrentHealth -= ActualDamage;

		// 🔑 重点：服务器手动调用一次，确保房主自己能看到 UI 更新
		UpdateHealthUI();

		// 死亡日志打印
		if (CurrentHealth <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("Character is DEAD!"));
		}
	}

	return ActualDamage;
}

// 客户端收到更新后打印一条日志
void AMultiplayerFPSDemoCharacter::OnRep_CurrentHealth()
{
	UpdateHealthUI();
	// 修正点：改用 LogTemp
	UE_LOG(LogTemp, Log, TEXT("Client: Health updated to %f"), CurrentHealth);
}