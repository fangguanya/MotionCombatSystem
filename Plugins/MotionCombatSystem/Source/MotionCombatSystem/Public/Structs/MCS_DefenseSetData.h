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
 * Date: 10-30-2025
 * =============================================================================
 * MCS_DefenseSetData.h
 * Represents a defense set in the MCS Combat System.
 */

#pragma once

#include "CoreMinimal.h"
#include <Choosers/MCS_DefenseChooser.h>
#include <Structs/MCS_DefenseEntry.h>
#include "MCS_DefenseSetData.generated.h"

 /**
  * @struct FMCS_DefenseSetData
  * @brief Represents a single Defense Set configuration within the Motion Combat System.
  *
  * Each defense set combines a Defense DataTable (containing FMCS_DefenseEntry rows)
  * and a Defense Chooser instance that dynamically selects the best defensive action.
  *
  * Designers can define multiple sets (e.g., Unarmed, SwordShield, DualBlades)
  * and switch between them at runtime via SetActiveDefenseSet().
  *
  * @see UMCS_CombatDefenseComponent
  * @see UMCS_DefenseChooser
  * @see FMCS_DefenseEntry
  */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Defense Set",
    ShortTooltip = "Defines a defensive data set with a linked chooser for the MCS Combat System."))
struct MOTIONCOMBATSYSTEM_API FMCS_DefenseSetData
{
    GENERATED_BODY()

public:

    /**
     * @brief DataTable containing all defense entries (FMCS_DefenseEntry) for this defense set.
     *
     * Each row defines a defensive action such as block, dodge, duck, roll, or parry.
     * The Defense Chooser will score and select entries from this table during gameplay.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Defense Data Table",
            ToolTip = "DataTable containing FMCS_DefenseEntry rows that define available defense actions.",
            RowType = "/Script/MotionCombatSystem/Structs/FMCS_DefenseEntry"))
    TObjectPtr<UDataTable> DefenseDataTable = nullptr;

    /**
     * @brief The Defense Chooser instance responsible for selecting the optimal defense action.
     *
     * Each Defense Set can have its own Chooser with unique scoring logic or weighting,
     * allowing specialized defensive behaviors per stance, weapon, or AI type.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense|Chooser",
        meta = (DisplayName = "Defense Chooser",
            ToolTip = "Defense Chooser instance that scores and selects defensive actions for this set."))
    TSubclassOf<UMCS_DefenseChooser> DefenseChooser = nullptr;
};
