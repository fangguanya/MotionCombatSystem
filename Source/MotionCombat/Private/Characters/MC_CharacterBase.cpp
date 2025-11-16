/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat System
 * This is a combat system inspired by Unreal Engine’s Motion Matching plugin.
 * Author: Christopher D. Parker
 * Date: 11-11-2025
 * =============================================================================
 * MC_CharacterBase.cpp
 * Base character class for Motion Combat.
 */

#include "Characters/MC_CharacterBase.h"


 // Sets default values
AMC_CharacterBase::AMC_CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the health component
	HealthComponent = CreateDefaultSubobject<UMC_HealthComponent>(TEXT("Health Component"));

	// Create and attach the combat core component
	CombatCoreComponent = CreateDefaultSubobject<UMCS_CombatCoreComponent>(TEXT("Combat Core Component"));

	// Create and attach the combat hitbox component
	CombatHitboxComponent = CreateDefaultSubobject<UMCS_CombatHitboxComponent>(TEXT("Combat Hitbox Component"));

	// Create and attach the combat defense component
	CombatDefenseComponent = CreateDefaultSubobject<UMCS_CombatDefenseComponent>(TEXT("Combat Defense Component"));

	// Create and attach the combat hit reaction component
	CombatHitReactionComponent = CreateDefaultSubobject<UMCS_CombatHitReactionComponent>(TEXT("Combat Hit Reaction Component"));
}

// Called when the game starts or when spawned
void AMC_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AMC_CharacterBase::OnCharacterHealthChanged);
		HealthComponent->OnDamageTaken.AddDynamic(this, &AMC_CharacterBase::OnCharacterDamageTaken);
		HealthComponent->OnDeath.AddDynamic(this, &AMC_CharacterBase::OnCharacterDeath);
	}
}

// Called every frame
void AMC_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/*
 * Called when the game ends or actor is destroyed
 * @param EndPlayReason The reason for ending play
 */
void AMC_CharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveAll(this);
		HealthComponent->OnDamageTaken.RemoveAll(this);
		HealthComponent->OnDeath.RemoveAll(this);
	}

	bIsAttacking = false;
	bIsDefending = false;
	bIsDead = false;

	Super::EndPlay(EndPlayReason);
}

// Called to bind functionality to input
void AMC_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

/*
 * Blueprint Native Event Implementations
 */

void AMC_CharacterBase::OnCharacterHealthChanged_Implementation(float NewHealth, float MaxHealth)
{
	// Default = do nothing (BP override handles behavior)
}

void AMC_CharacterBase::OnCharacterDamageTaken_Implementation(float DamageAmount, float NewHealth, AActor* InstigatorActor)
{
	// Default = do nothing
}

void AMC_CharacterBase::OnCharacterDeath_Implementation(AActor* DeadActor)
{
	// Default = do nothing
}
