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
 * Date: 11-15-2025
 * =============================================================================
 * MC_HealthComponent.cpp
 * Implements health, damage, healing, and death events for MCS actors.
 */

#include "Components/MC_HealthComponent.h"
#include "GameFramework/Actor.h"


UMC_HealthComponent::UMC_HealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMC_HealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    bIsDead = false;
}

void UMC_HealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    OnHealthChanged.Clear();
    OnDamageTaken.Clear();
    OnDeath.Clear();

    Super::EndPlay(EndPlayReason);
}

void UMC_HealthComponent::ClampHealth()
{
    CurrentHealth = FMath::Clamp(CurrentHealth, 0.f, MaxHealth);
}

void UMC_HealthComponent::ApplyDamage(float DamageAmount, AActor* InstigatorActor)
{
    if (bIsDead || DamageAmount <= 0.f)
        return;

    CurrentHealth -= DamageAmount;
    ClampHealth();

    OnDamageTaken.Broadcast(DamageAmount, CurrentHealth, InstigatorActor);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        HandleDeath(InstigatorActor);
    }
}

void UMC_HealthComponent::Heal(float HealAmount)
{
    if (bIsDead || HealAmount <= 0.f)
        return;

    CurrentHealth += HealAmount;
    ClampHealth();

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UMC_HealthComponent::RestoreToFull()
{
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UMC_HealthComponent::Kill(AActor* InstigatorActor)
{
    if (bIsDead)
        return;

    CurrentHealth = 0.f;
    ClampHealth();
    HandleDeath(InstigatorActor);
}

void UMC_HealthComponent::HandleDeath(AActor* InstigatorActor)
{
    if (bIsDead)
        return;

    bIsDead = true;

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    OnDeath.Broadcast(GetOwner());
}
