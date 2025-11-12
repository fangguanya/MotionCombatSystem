/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat System
 * Author: Christopher D. Parker
 * Date: 11-11-2025
 * =============================================================================
 * MC_AIStateEnums.h
 * Declares core enums used by the Motion Combat AI logic system.
 */

#pragma once

#include "CoreMinimal.h"
#include "MC_AIStateEnums.generated.h"

/**
 * Primary AI state (high-level logic)
 */
UENUM(BlueprintType, meta=(DisplayName="Motion Combat AI State"))
enum class EMC_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Combat      UMETA(DisplayName = "Combat"),
    Recover     UMETA(DisplayName = "Recover"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * Optional AI substate (micro behavior inside a major state)
 * Example: within Combat, substate could be Engage or Attack.
 */
UENUM(BlueprintType, meta=(DisplayName="Motion Combat AI State"))
enum class EMC_AISubState : uint8
{
    None        UMETA(DisplayName = "None"),
    Engage      UMETA(DisplayName = "Engage"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
};

/**
 * Event types that can trigger state transitions.
 * Useful for driving AI from perception, combat, or animation events.
 */
UENUM(BlueprintType, meta=(DisplayName="Motion Combat AI Event"))
enum class EMC_AIEvent : uint8
{
    None                UMETA(DisplayName = "None"),
    OnSeeEnemy          UMETA(DisplayName = "On See Enemy"),
    OnLostEnemy         UMETA(DisplayName = "On Lost Enemy"),
    OnTakeDamage        UMETA(DisplayName = "On Take Damage"),
    OnHearNoise         UMETA(DisplayName = "On Hear Noise"),
    OnDeath             UMETA(DisplayName = "On Death"),
};
