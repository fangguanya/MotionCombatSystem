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
 * MCS_DefenseChooser.cpp
 * Implementation of the Defense Chooser logic, handling contextual scoring
 * and flexible designer-overridable validation for defensive actions.
 */

#include <Choosers/MCS_DefenseChooser.h>
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"


/**
 * @brief Constructor for UMCS_DefenseChooser.
 */
UMCS_DefenseChooser::UMCS_DefenseChooser(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Constructor logic (if any) goes here
}

/**
 * @brief Evaluates all defensive entries and selects the one with the highest score.
 *
 * This function iterates through each available FMCS_DefenseEntry in DefenseEntries.
 * It calls CanAttemptDefense() to ensure each entry is eligible, then uses ScoreDefense()
 * to calculate a suitability score. The entry with the highest score is chosen as the optimal defense.
 *
 * @param Defender         The actor performing the defensive action.
 * @param Attacker         The actor initiating the attack.
 * @param Intent           The defense intent (Defense or Parry).
 * @param OutChosenEntry   [Out] The resulting defense entry selected by the chooser.
 * @return True if a valid defense was found; false otherwise.
 */
bool UMCS_DefenseChooser::ChooseDefense(AActor* Defender, AActor* Attacker, EMCS_DefenseIntent Intent, FMCS_DefenseEntry& OutChosenEntry)
{
    if (!Defender || DefenseEntries.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[DefenseChooser] Invalid input or no DefenseEntries available."));
        return false;
    }

    float BestScore = -FLT_MAX;
    bool bFound = false;

    for (const FMCS_DefenseEntry& Entry : DefenseEntries)
    {
        // Skip if not eligible for attempt
        if (!CanAttemptDefense(Entry, Defender, Attacker))
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("[DefenseChooser] Skipping %s (CanAttemptDefense returned false)"), *Entry.DefenseName.ToString());
            continue;
        }

        const float Score = ScoreDefense(Entry, Defender, Attacker, Intent);
        if (Score > BestScore)
        {
            BestScore = Score;
            OutChosenEntry = Entry;
            bFound = true;
        }
    }

    if (bFound)
    {
        UE_LOG(LogTemp, Log, TEXT("[DefenseChooser] Selected Defense: %s | Score: %.2f"), *OutChosenEntry.DefenseName.ToString(), BestScore);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[DefenseChooser] No valid defense found."));
    }

    return bFound;
}

/**
 * @brief Calculates the total score for a defense entry based on context.
 *
 * The default implementation adds scoring contributions from:
 * - Intent matching
 * - Distance scoring (ScoreDistance)
 * - Facing alignment (ScoreFacing)
 * - Random variation for unpredictability
 *
 * Override this function in Blueprint or C++ subclasses to extend scoring logic.
 *
 * @param Entry      The defense entry being evaluated.
 * @param Defender   The actor performing the defense.
 * @param Attacker   The actor initiating the attack.
 * @param Intent     The defense intent (Defense or Parry) guiding selection.
 * @return A float value representing the total suitability score of the entry.
 */
float UMCS_DefenseChooser::ScoreDefense_Implementation(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker, EMCS_DefenseIntent Intent) const
{
    if (!IsValid(Defender) || !IsValid(Attacker))
        return -FLT_MAX;

    float TotalScore = 0.0f;

    // Intent match bonus
    if (Entry.DefenseIntent == Intent)
    {
        TotalScore += 50.f;
    }
    else
    {
        TotalScore -= 25.f;
    }

    // Distance contribution
    TotalScore += ScoreDistance(Entry, Defender, Attacker);

    // Facing contribution
    TotalScore += ScoreFacing(Entry, Defender, Attacker);

    // Add slight random variation to reduce predictable repetition
    TotalScore += FMath::FRandRange(-5.f, 5.f);

    return TotalScore;
}

/**
 * @brief Calculates a smooth distance-based score for a defense entry.
 *
 * Instead of a hard cutoff, this version gradually reduces score as distance
 * moves away from the ideal range midpoint, producing more natural AI decisions.
 *
 * @param Entry      The defense entry containing valid distance range (Range.X - Range.Y).
 * @param Defender   The actor performing the defense.
 * @param Attacker   The actor initiating the attack.
 * @return A float score between -25 and +25 based on distance suitability.
 */
float UMCS_DefenseChooser::ScoreDistance(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const
{
    if (!IsValid(Defender) || !IsValid(Attacker))
        return 0.0f;

    const float Dist = FVector::Dist(Defender->GetActorLocation(), Attacker->GetActorLocation());

    // Range midpoint and half-width
    const float MidRange = (Entry.Range.X + Entry.Range.Y) * 0.5f;
    const float RangeExtent = (Entry.Range.Y - Entry.Range.X) * 0.5f;

    // If no meaningful range, skip
    if (RangeExtent <= 1.f)
        return 0.0f;

    // Normalized score factor (1 = perfect, 0 = far outside)
    const float Normalized =
        FMath::Clamp(1.f - FMath::Abs(Dist - MidRange) / RangeExtent, 0.f, 1.f);

    // Scale to ±25 scoring window
    const float Score = (Normalized * 50.f) - 25.f; // -25 at edge, +25 at perfect distance

    return Score;
}

/**
 * @brief Evaluates a defense entry based on defender’s facing direction relative to attacker.
 *
 * This scoring helps emphasize actions like blocking and parrying, which require proper orientation
 * toward the attacker. Forward alignment increases the score; misalignment results in no bonus.
 *
 * @param Entry      The defense entry containing direction data.
 * @param Defender   The actor performing the defense.
 * @param Attacker   The actor initiating the attack.
 * @return A float score (typically +10 if correctly facing; otherwise 0).
 */
float UMCS_DefenseChooser::ScoreFacing(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const
{
    if (!IsValid(Defender) || !IsValid(Attacker))
        return 0.0f;

    const FVector ToAttacker = (Attacker->GetActorLocation() - Defender->GetActorLocation()).GetSafeNormal();
    const float Facing = FVector::DotProduct(Defender->GetActorForwardVector(), ToAttacker);

    if (Entry.ValidDirection == EMCS_AttackDirection::Forward && Facing > 0.25f)
    {
        return 10.f;
    }
    return 0.0f;
}

/**
 * @brief Determines if a specific defense action is currently valid for execution.
 *
 * This method serves as a filter for eligibility—returning true allows the chooser
 * to consider the entry for scoring, while false excludes it. Override in Blueprints
 * or C++ to apply stamina checks, cooldown restrictions, or state validation.
 *
 * @param Entry      The defense entry being validated.
 * @param Defender   The actor attempting to perform the defense.
 * @param Attacker   The actor initiating the attack.
 * @return True if the defense entry can be attempted; false if restricted.
 */
bool UMCS_DefenseChooser::CanAttemptDefense_Implementation(const FMCS_DefenseEntry& Entry, AActor* Defender, AActor* Attacker) const
{
    // Default: always permitted.
    // Designers may override this logic in Blueprint/C++ for stamina or cooldown restrictions.
    return true;
}

/**
 * Visualizes distance-based defense scoring for debugging.
 */
void UMCS_DefenseChooser::DrawDebugDistanceScores(AActor* Defender, AActor* Attacker, float Duration) const
{
    if (!IsValid(Defender) || !IsValid(Attacker))
        return;

    const FVector DefenderLoc = Defender->GetActorLocation();
    const FVector AttackerLoc = Attacker->GetActorLocation();
    const float ActualDist = FVector::Dist(DefenderLoc, AttackerLoc);

    // Draw a line between defender and attacker
    DrawDebugLine(
        Defender->GetWorld(),
        DefenderLoc + FVector(0, 0, 50),
        AttackerLoc + FVector(0, 0, 50),
        FColor::White,
        false,
        Duration,
        0,
        1.5f
    );

    // Display actual distance
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            Duration,
            FColor::White,
            FString::Printf(TEXT("Defense Distance: %.1f cm"), ActualDist));
    }

    // Iterate over all defense entries to visualize score by range
    for (const FMCS_DefenseEntry& Entry : DefenseEntries)
    {
        const float Score = ScoreDistance(Entry, Defender, Attacker);

        // Compute color based on score
        FColor ScoreColor = FColor::MakeRedToGreenColorFromScalar((Score + 25.f) / 50.f);

        const FVector TextPos = DefenderLoc + FVector(0, 0, 100 + (&Entry - DefenseEntries.GetData()) * 15);

        // Draw entry name + score
        if (GEngine)
        {
            const FString DebugText = FString::Printf(
                TEXT("[%s] Score: %.1f | Range %.0f–%.0f cm"),
                *Entry.DefenseName.ToString(),
                Score,
                Entry.Range.X,
                Entry.Range.Y);

            GEngine->AddOnScreenDebugMessage(-1, Duration, ScoreColor, DebugText);
        }

        // Optional: world-space label above defender
        DrawDebugString(
            Defender->GetWorld(),
            TextPos,
            FString::Printf(TEXT("%s (%.1f)"), *Entry.DefenseName.ToString(), Score),
            nullptr,
            ScoreColor,
            Duration,
            false
        );
    }
}
