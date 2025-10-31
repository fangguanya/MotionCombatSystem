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
 * Date: 10-27-2025
 * =============================================================================
 * MCS_DefenseEntry.h
 * Defines the Motion Combat System Defense Entry — a single defensive action entry
 * used by the Defense Chooser for contextual scoring and selection.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "DataTableUtils.h"
#include <Enums/EMCS_AttackDirections.h>
#include "MCS_DefenseEntry.generated.h"

 /**
  * @enum EMCS_DefenseIntent
  * @brief Defines the high-level intent for a defensive action.
  *
  * This helps the Defense Chooser distinguish between general defense actions
  * (block, dodge, duck, roll) and high-risk, high-reward parry actions.
  */
UENUM(BlueprintType, meta = (DisplayName = "Motion Combat System Defense Intent",
    ToolTip = "Defines the high-level intent for a defensive action."))
    enum class EMCS_DefenseIntent : uint8
{
    /** Standard defensive maneuvers such as block, dodge, duck, or roll. */
    Defense	UMETA(DisplayName = "Defense (Block, Dodge, Duck, Roll)"),

    /** High-risk, high-reward defensive actions like parrying. */
    Parry	UMETA(DisplayName = "Parry (High Risk / High Reward)")
};

/**
 * @struct FMCS_DefenseEntry
 * @brief Represents a single defensive action that can be chosen by the Defense Chooser.
 *
 * Each entry defines one possible defensive response — for example, a forward block,
 * a side roll, or a parry montage. These entries are evaluated and scored dynamically
 * based on context, such as distance, direction, and defensive intent.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Motion Combat System Defense Entry",
    ShortTooltip = "Defines a single defensive action entry for scoring and selection."))
    struct FMCS_DefenseEntry : public FTableRowBase
{
    GENERATED_BODY();

    /* =====================================================================
     * General
     * ===================================================================== */

     /**
      * @brief A designer-friendly name to identify this defense entry.
      * Typically used as a label within data tables or debugging output.
      */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Defense Name",
            ToolTip = "A designer-friendly label identifying this defense entry."))
    FName DefenseName;

    /**
     * @brief Optional category grouping for organizational purposes (e.g., 'Ground', 'Air', 'Special').
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Category",
            ToolTip = "Optional grouping label for categorizing defense entries (e.g., Ground, Air, or Special)."))
    FName Category;

    /**
     * @brief The defense intent for this entry — either general defense or parry.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Defense Intent",
            ToolTip = "Defines whether this entry is a regular defensive move or a parry-type action."))
    EMCS_DefenseIntent DefenseIntent = EMCS_DefenseIntent::Defense;

    /* =====================================================================
     * Montage and Tags
     * ===================================================================== */

     /**
      * @brief The specific gameplay tag describing this defensive action.
      * Example: 'MCS.Defense.Block' or 'MCS.Defense.Roll'.
      */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Action Tag",
            ToolTip = "Gameplay Tag identifying the type of defensive action (e.g., MCS.Defense.Block)."))
    FGameplayTag ActionTag;

    /**
     * @brief The animation montage to play when this defense entry is executed.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Defense Montage",
            ToolTip = "The animation montage played when this defensive action is chosen."))
    TObjectPtr<UAnimMontage> DefenseMontage = nullptr;

    /**
     * @brief Optional montage section to begin playback from.
     * If left empty, playback begins from the montage's default starting section.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Montage Section",
            ToolTip = "Optional section within the Defense Montage to start playback from."))
    FName MontageSection = NAME_None;

    /* =====================================================================
     * Scoring and Selection
     * ===================================================================== */

     /**
      * @brief The weight value used during scoring tie-breaks and randomization.
      * Higher values make this entry more likely to be chosen when scores are equal.
      */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Selection Weight",
            ToolTip = "Relative weight used for random/weighted selection when multiple defenses score equally."))
    float SelectionWeight = 1.0f;

    /**
     * @brief The attack direction in which this defense entry is valid.
     * For example, 'Forward' blocks may only protect against frontal attacks.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Valid Direction",
            ToolTip = "Specifies the attack direction this defense is effective against (e.g., Forward, Left, Omni)."))
    EMCS_AttackDirection ValidDirection = EMCS_AttackDirection::Omni;

    /**
     * @brief The distance range from the attacker where this defense is effective.
     * Used in distance-based scoring to ensure appropriate contextual use.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Effective Range (cm)",
            ToolTip = "The valid distance range (in cm) where this defense can be used effectively."))
    FVector2D Range = FVector2D(0.f, 1000.f);

    /* =====================================================================
     * Tag Filtering
     * ===================================================================== */

     /**
      * @brief Gameplay Tags required for this defense to be available.
      * Example: Character must have 'MCS.State.Grounded'.
      */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Required Tags",
            ToolTip = "Tags that must be present on the defender to enable this defense (e.g., MCS.State.Grounded)."))
    FGameplayTagContainer RequiredTags;

    /**
     * @brief Gameplay Tags that, if present, will prevent this defense from being used.
     * Example: Character has 'MCS.State.Stunned'.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Excluded Tags",
            ToolTip = "Tags that disable this defense if the defender currently has them (e.g., MCS.State.Stunned)."))
    FGameplayTagContainer ExcludedTags;
};
