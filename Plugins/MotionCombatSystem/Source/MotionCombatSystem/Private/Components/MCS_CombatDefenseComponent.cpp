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

void UMCS_CombatDefenseComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (Owner)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatDefense] Initialized for Actor: %s"), *Owner->GetName());
    }
}
