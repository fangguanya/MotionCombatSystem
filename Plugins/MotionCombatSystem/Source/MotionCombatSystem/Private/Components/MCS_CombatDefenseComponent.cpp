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
 * Date: 10-26-2025
 * =============================================================================
 * MCS_CombatDefenseComponent.cpp
 * Implementation of the skeleton Defense Component for the Motion Combat System.
 */

#include <Components/MCS_CombatDefenseComponent.h>
#include "GameFramework/Actor.h"

UMCS_CombatDefenseComponent::UMCS_CombatDefenseComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

/**
 * Called when the game starts
 */
void UMCS_CombatDefenseComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        if (UMCS_CombatCoreComponent* Core = Owner->FindComponentByClass<UMCS_CombatCoreComponent>())
        {
            // Bind to defense events
            Core->OnDefenseWindowBegin.AddDynamic(this, &UMCS_CombatDefenseComponent::HandleDefenseWindowBegin);
            Core->OnDefenseWindowEnd.AddDynamic(this, &UMCS_CombatDefenseComponent::HandleDefenseWindowEnd);

            // Bind to parry events
            Core->OnParryWindowBegin.AddDynamic(this, &UMCS_CombatDefenseComponent::HandleParryWindowBegin);
            Core->OnParryWindowEnd.AddDynamic(this, &UMCS_CombatDefenseComponent::HandleParryWindowEnd);
        }

        UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Initialized for Actor: %s"), *Owner->GetName());
    }
}

/**
 * Called when the game ends
 */
void UMCS_CombatDefenseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // Unbind from combat events
    if (AActor* Owner = GetOwner())
    {
        if (UMCS_CombatCoreComponent* Core = Owner->FindComponentByClass<UMCS_CombatCoreComponent>())
        {
            // Unbind from defense events
            Core->OnDefenseWindowBegin.RemoveDynamic(this, &UMCS_CombatDefenseComponent::HandleDefenseWindowBegin);
            Core->OnDefenseWindowEnd.RemoveDynamic(this, &UMCS_CombatDefenseComponent::HandleDefenseWindowEnd);

            // Unbind from parry events
            Core->OnParryWindowBegin.RemoveDynamic(this, &UMCS_CombatDefenseComponent::HandleParryWindowBegin);
            Core->OnParryWindowEnd.RemoveDynamic(this, &UMCS_CombatDefenseComponent::HandleParryWindowEnd);
        }
    }
}

/**
 * Handle Parry Window Begin
 * @param Attacker - the actor who initiated the parry window
 */
void UMCS_CombatDefenseComponent::HandleParryWindowBegin(AActor* Attacker)
{
    bIsInParryWindow = true;
    LastParrySource = Attacker;
    OnParryOpportunity.Broadcast(Attacker);

    UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Parry window OPEN from %s"), *GetNameSafe(Attacker));
}

/**
 * Handle Parry Window End
 * @param Attacker - the actor who initiated the parry window
 */
void UMCS_CombatDefenseComponent::HandleParryWindowEnd(AActor* Attacker)
{
    bIsInParryWindow = false;
    LastParrySource = nullptr;

    UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Parry window CLOSED from %s"), *GetNameSafe(Attacker));
}

/**
 * Handle Defense Window Begin
 * @param Defender - the actor who is defending
 */
void UMCS_CombatDefenseComponent::HandleDefenseWindowBegin(AActor* Defender)
{
    bIsInDefenseWindow = true;
    OnDefenseWindowActive.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Defense Window OPEN (Defender: %s)"), *GetNameSafe(Defender));
}

/**
 * Handle Defense Window End
 * @param Defender - the actor who is defending
 */
void UMCS_CombatDefenseComponent::HandleDefenseWindowEnd(AActor* Defender)
{
    bIsInDefenseWindow = false;
    UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Defense Window CLOSED (Defender: %s)"), *GetNameSafe(Defender));
}

bool UMCS_CombatDefenseComponent::TryParry()
{
    // ✅ Can only parry if a window is active and we have a valid attacker
    if (!bIsInParryWindow || !IsValid(LastParrySource))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatDefense] Parry failed: No active window or invalid source."));
        OnParryFail.Broadcast();
        return false;
    }

    // Later we’ll add conditions like facing, stamina, reaction time, etc.
    const FVector ToAttacker = (LastParrySource->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
    const FVector Forward = GetOwner()->GetActorForwardVector();

    const float FacingDot = FMath::Clamp(FVector::DotProduct(Forward, ToAttacker), -1.f, 1.f); // -1 to 1
    const bool bIsFacingAttacker = FacingDot > 0.25f; // ~75° cone

    if (bIsFacingAttacker)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatDefense] Parry SUCCESS against %s"), *GetNameSafe(LastParrySource));
        OnParrySuccess.Broadcast();
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("[CombatDefense] Parry FAILED (not facing attacker)."));
    OnParryFail.Broadcast();
    return false;
}

bool UMCS_CombatDefenseComponent::TryBlock()
{
    if (!bIsInDefenseWindow)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatDefense] Block failed: No active defense window."));
        OnBlockFail.Broadcast();
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("[CombatDefense] Block SUCCESS."));
    OnBlockSuccess.Broadcast();
    return true;
}
