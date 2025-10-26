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
 * Date: 10-13-2025
 * =============================================================================
 * MCS_CombatTargetInterface.h
 * Interface that handles various aspects of Characters using the Motion Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include <Structs/MCS_AttackEntry.h>
#include "MCS_CombatTargetInterface.generated.h"

 // This macro creates the interface’s UClass type
UINTERFACE(Blueprintable, meta = (DisplayName = "Motion Combat System Character Interface"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatCharacterInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Interface for characters that are a part of the Motion Combat System.
 */
class MOTIONCOMBATSYSTEM_API IMCS_CombatCharacterInterface
{
    GENERATED_BODY()

public:
    
    /**
     * Whether this actor can currently be targeted (true = valid target).
     * @return True if the actor can be targeted, false otherwise.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Target",
        meta = (DisplayName = "Can Be Targeted", Tooltip = "Whether this actor can currently be targeted (true = valid target)."))
    bool CanBeTargeted() const;
    virtual bool CanBeTargeted_Implementation() const { return true; }

    /**
     * Apply combat damage to this actor.
     * @param Damage The amount of damage to apply.
     * @param HitLocation The location where the hit occurred.
     * @param AttackEntry The attack entry details associated with the damage.
     * @return True if damage was successfully applied, false otherwise.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat Damage",
        meta = (
            DisplayName = "Take Combat Damage",
            Tooltip = "Applies combat damage to this actor using data from a Motion Combat attack entry.",
            DisplayName_Damage = "Damage Amount",
            Tooltip_Damage = "The total amount of damage dealt by the incoming attack.",
            DisplayName_HitLocation = "Hit result",
            Tooltip_HitLocation = "The hit result containing impact point and bone information.",
            DisplayName_AttackEntry = "Attack Entry",
            Tooltip_AttackEntry = "The Motion Combat attack data (montage, tags, severity, etc.) associated with this hit.")
    )
    bool TakeCombatDamage(float Damage, const FHitResult& Hit, const FMCS_AttackEntry& AttackEntry) const;
};
