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
 * Date: 10-14-2025
 * =============================================================================
 * MCS_AttackSetData.h
 * Defines a Motion Combat System Attack Set — linking a DataTable of attack entries
 * with a specific Attack Chooser for dynamic selection during combat.
 */

#pragma once

#include "CoreMinimal.h"
#include <Choosers/MCS_AttackChooser.h>
#include <Structs/MCS_AttackEntry.h>
#include "MCS_AttackSetData.generated.h"

 /**
  * @struct FMCS_AttackSetData
  * @brief Represents a single Attack Set configuration within the Motion Combat System.
  *
  * Each Attack Set combines a DataTable (containing FMCS_AttackEntry rows)
  * with a Chooser instance that dynamically selects the most appropriate attack
  * based on current combat context, player input, and design intent.
  *
  * Designers can create multiple Attack Sets (e.g., Unarmed, Sword, Dual Blades)
  * and switch between them dynamically during gameplay to reflect stance, weapon,
  * or character state.
  *
  * @see UMCS_CombatCoreComponent
  * @see UMCS_AttackChooser
  * @see FMCS_AttackEntry
  */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Attack Set",
    ShortTooltip = "Defines an attack data set with a linked chooser for the Motion Combat System."))
struct MOTIONCOMBATSYSTEM_API FMCS_AttackSetData
{
    GENERATED_BODY()

public:

    /**
     * @brief The DataTable containing attack entries (FMCS_AttackEntry) for this attack set.
     *
     * Each row in this table defines an attack action, including range, direction, weight,
     * and other combat metadata. The linked Attack Chooser uses this table to dynamically
     * score and select the most contextually appropriate attack.
     *
     * Example:
     * - Light_AttackSet_DataTable
     * - Heavy_AttackSet_DataTable
     * - DualBlades_ComboAttackTable
     *
     * @details The DataTable must use the FMCS_AttackEntry row structure.
     *
     * @see FMCS_AttackEntry
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Combat|Attack",
        meta = (DisplayName = "Attack Data Table",
            ToolTip = "DataTable containing FMCS_AttackEntry rows defining available attacks for this set.",
            RowType = "/Script/MotionCombatSystem/Structs/FMCS_AttackEntry"))
    TObjectPtr<UDataTable> AttackDataTable = nullptr;

    /**
     * @brief The Attack Chooser instance used for this Attack Set.
     *
     * The chooser is responsible for evaluating and scoring attacks from the
     * linked DataTable based on game-specific rules such as player intent,
     * distance to target, combo state, and attack direction.
     *
     * Designers can assign custom Blueprint subclasses of UMCS_AttackChooser
     * to create unique scoring logic or behavior per weapon or stance.
     *
     * @details This property is Instanced, allowing designers to modify
     * chooser settings per attack set directly inside the editor.
     *
     * @see UMCS_AttackChooser
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Combat|Chooser",
        meta = (DisplayName = "Attack Chooser",
            ToolTip = "Attack Chooser instance that scores and selects attacks for this attack set."))
    TSubclassOf<UMCS_AttackChooser> AttackChooser = nullptr;
};
