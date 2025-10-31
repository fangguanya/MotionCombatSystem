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
 * MCS_DefenseChooser.h
 * Handles dynamic defensive action selection (block, dodge, roll, parry, etc.)
 * based on contextual scoring and designer intent.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include <Enums/EMCS_AttackDirections.h>
#include <Structs/MCS_DefenseEntry.h>
#include <Structs/MCS_DebugInfo.h>
#include "MCS_DefenseChooser.generated.h"


/**
 * The Defense Chooser is responsible for selecting the optimal defensive action
 * based on contextual scoring, intent, and optional custom eligibility logic.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (MotionCombatSystem),
    meta = (DisplayName = "Motion Combat System Defense Chooser", ShortTooltip = "Chooses the best defensive action based on scoring and intent.")
)
class MOTIONCOMBATSYSTEM_API UMCS_DefenseChooser : public UObject
{
    GENERATED_BODY()

public:
    UMCS_DefenseChooser(const FObjectInitializer& ObjectInitializer);

    /* ==========================================================
     * Configurable Data
     * ========================================================== */
    
     /*
     * Properties
    */
    
    /**
     * @brief The list of available defensive actions (entries) that this chooser can evaluate.
     * Each entry defines a possible defensive response, such as a block, dodge, roll, or parry.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense",
        meta = (DisplayName = "Defense Entries", ToolTip = "List of available defensive actions to choose from. Each entry defines a block, dodge, roll, or parry option."))
    TArray<FMCS_DefenseEntry> DefenseEntries;

    /* ==========================================================
     * Public API
     * ========================================================== */

    /**
     * @brief Calculates the total score for a given defense entry.
     * This function determines how suitable a particular defensive action is given the current combat context.
     * Can be overridden in Blueprint or C++ subclasses for custom logic.
     *
     * @param Entry      The defense entry being evaluated.
     * @param Defender   The actor performing the defensive action.
     * @param Attacker   The actor initiating the attack.
     * @param Intent     The defense intent (Defense or Parry) indicating the desired defensive behavior.
     * @return A float representing the total suitability score of this defense entry.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "MCS|Defense",
        meta = (DisplayName = "Score Defense",
            ReturnDisplayName = "Score",
            ToolTip = "Calculates a total score for a defense entry based on context.",
            AdvancedDisplay = "Intent",
            AutoCreateRefTerm = "Entry",
            DisplayNameParam = "Entry, Defender, Attacker, Intent",
            DisplayNameEntry = "Defense Entry",
            DisplayNameDefender = "Defender",
            DisplayNameAttacker = "Attacker",
            DisplayNameIntent = "Intent"))
    float ScoreDefense(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker, EMCS_DefenseIntent Intent) const;
    virtual float ScoreDefense_Implementation(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker, EMCS_DefenseIntent Intent) const;

    /**
     * @brief Selects the best defensive entry from the available list, based on scoring results.
     * It evaluates all DefenseEntries, applies eligibility checks via CanAttemptDefense(),
     * and chooses the entry with the highest total score.
     *
     * @param Defender        The actor performing the defense.
     * @param Attacker        The actor initiating the attack.
     * @param Intent          The defensive intent (Defense or Parry).
     * @param OutChosenEntry  The resulting defense entry that was selected.
     * @return True if a valid defensive entry was found; false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Defense",
        meta = (DisplayName = "Choose Defense",
            ReturnDisplayName = "Was Defense Chosen",
            ToolTip = "Evaluates all defensive options and selects the most appropriate entry.",
            DisplayNameDefender = "Defender",
            DisplayNameAttacker = "Attacker",
            DisplayNameIntent = "Intent",
            DisplayNameOutChosenEntry = "Chosen Entry"))
    bool ChooseDefense(AActor* Defender, AActor* Attacker, EMCS_DefenseIntent Intent, FMCS_DefenseEntry& OutChosenEntry);

    /**
     * @brief Determines whether the specified defense entry can be attempted by the defender.
     * This allows designers or programmers to impose custom restrictions (e.g., stamina checks, cooldowns, conditions).
     * The default implementation always returns true.
     *
     * @param Entry      The defense entry being evaluated.
     * @param Defender   The actor attempting the defense.
     * @param Attacker   The actor initiating the attack.
     * @return True if the defense entry can be attempted; false otherwise.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "MCS|Defense",
        meta = (DisplayName = "Can Attempt Defense",
            ReturnDisplayName = "Can Attempt",
            ToolTip = "Determines if a defense entry can be attempted. Override for stamina, cooldown, or state restrictions.",
            DisplayNameEntry = "Defense Entry",
            DisplayNameDefender = "Defender",
            DisplayNameAttacker = "Attacker"))
    bool CanAttemptDefense(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const;
    virtual bool CanAttemptDefense_Implementation(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const;

    /**
     * @brief Calculates the distance score between the defender and the attacker.
     * Returns a positive score if the attacker is within the valid distance range specified in the defense entry.
     *
     * @param Entry      The defense entry containing range data.
     * @param Defender   The actor performing the defense.
     * @param Attacker   The actor initiating the attack.
     * @return A float between -25 and +25 based on distance suitability.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Defense|Scoring",
        meta = (DisplayName = "Score Distance",
            ToolTip = "Scores a defensive action based on distance between defender and attacker.",
            DisplayNameEntry = "Defense Entry",
            DisplayNameDefender = "Defender",
            DisplayNameAttacker = "Attacker",
            ReturnDisplayName = "Distance Score"))
    float ScoreDistance(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const;

    /**
     * @brief Calculates the facing score between the defender and the attacker.
     * Returns a bonus if the defender is facing the attacker within the valid direction defined by the entry.
     *
     * @param Entry      The defense entry containing valid direction data.
     * @param Defender   The actor performing the defense.
     * @param Attacker   The actor initiating the attack.
     * @return A float bonus based on directional alignment (typically +10 for correct facing).
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Defense|Scoring",
        meta = (DisplayName = "Score Facing",
            ToolTip = "Scores a defensive action based on defender’s facing direction relative to the attacker.",
            DisplayNameEntry = "Defense Entry",
            DisplayNameDefender = "Defender",
            DisplayNameAttacker = "Attacker",
            ReturnDisplayName = "Facing Score"))
    float ScoreFacing(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const;

    /**
     * @brief Draws debug information for distance scoring on screen or in world space.
     *
     * This function visualizes the distance between Defender and Attacker
     * and shows the computed ScoreDistance() value per DefenseEntry.
     *
     * @param Defender   The actor performing the defense.
     * @param Attacker   The attacking actor.
     * @param Duration   Duration (in seconds) the debug info remains on screen.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Defense|Debug")
    void DrawDebugDistanceScores(AActor* Defender, AActor* Attacker, float Duration = 1.5f) const;
};
