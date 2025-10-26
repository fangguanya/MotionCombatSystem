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
 * MCS_CombatDefenseComponent.h
 * Skeleton component responsible for handling defensive actions such as
 * blocking, dodging, ducking, rolling, and defensive reaction montages.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCS_CombatDefenseComponent.generated.h"

 /**
  * UMCS_CombatDefenseComponent
  *
  * Component that will handle all player and AI defensive actions.
  * For now, it contains only basic setup and lifecycle hooks.
  */
UCLASS(Blueprintable, ClassGroup = (MotionCombat), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Defense Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatDefenseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatDefenseComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
};
