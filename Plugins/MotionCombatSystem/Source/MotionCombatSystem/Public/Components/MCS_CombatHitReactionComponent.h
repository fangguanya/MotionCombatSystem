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
 * Date: 10-18-2025
 * =============================================================================
 * MCS_CombatHitReactionComponent.h
 *
 * Component responsible for handling hit reactions (e.g. playing
 * reaction montages or effects) when the owning actor takes damage or is hit.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Structs/MCS_HitReaction.h>
#include "MCS_CombatHitReactionComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat Hit Reaction Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatHitReactionComponent();

    // Destructor
    virtual ~UMCS_CombatHitReactionComponent() override = default;

    /*
    * Functions
    */

    /**
     * Performs a hit reaction using hit result data.
     * Automatically determines direction from the hit location and selects
     * a matching reaction montage based on bone, region, direction, and severity.
     *
     * @param Hit - The hit result data containing impact point and bone info.
     * @param TargetActor - The actor that was hit (usually GetOwner()).
     * @param Severity - The hit severity (Light, Heavy, Knockback, etc.).
     */
    UFUNCTION(BlueprintCallable, Category = "Hit Reaction",
        meta = (
            DisplayName = "Perform Hit Reaction (Hit Result)",
            Tooltip = "Performs a hit reaction using a hit result. Uses bone and direction information to find the best reaction montage.",
            DisplayName_Hit = "Hit Result",
            Tooltip_Hit = "The hit result containing impact and bone information.",
            DisplayName_TargetActor = "Victim Actor",
            Tooltip_TargetActor = "The actor that received the hit (defaults to owner).",
            DisplayName_Severity = "Hit Severity",
            Tooltip_Severity = "The severity of the incoming attack."
            ))
    void PerformHitReaction(const FHitResult& Hit, AActor* TargetActor, EPGAS_HitSeverity Severity);
   
    /*
     * Properties
     */

     /** DataTable containing FMCS_HitReaction definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core", meta = (DisplayName = "Hit Reaction Data Table", RowType = "FMCS_HitReaction", Tooltip = "DataTable defining hit reaction montages based on direction and severity."))
    TObjectPtr<UDataTable> HitReactionDataTable;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:

    /*
     * Functions
     */

    /** Helper: Plays a montage on the owning actor's mesh if valid */
    void PlayMontageInternal(UAnimMontage* Montage, float InPlayRate = 1.0f);

    /**
     * Finds the best matching hit reaction using bone, region, direction, and severity hierarchy.
     * @param BoneName - The bone struck (can be NAME_None).
     * @param Direction - The direction of the hit (Forward, Back, etc.).
     * @param Severity - The severity of the hit (Light, Heavy, Knockdown, etc.).
     * @return The best matching hit reaction, or nullptr if none found.
     */
    const FMCS_HitReaction* FindReaction(const FName& BoneName, EMCS_Direction Direction, EPGAS_HitSeverity Severity) const;

    /**
     * Calculates hit direction based on hit location relative to the target actor.
     * @param HitLocation The world location of the hit.
     * @param TargetActor The actor that was hit.
     * @return The calculated hit direction enum value.
     */
    // UFUNCTION(BlueprintPure, Category = "Hit Reaction", meta = (DisplayName = "Get Hit Direction from Location"))
    static EMCS_Direction CalculateHitDirection(const FVector& HitLocation, AActor* TargetActor);

    /**
     * Internal helper that maps bone names to simplified body regions (e.g., "LegLeft", "Torso").
     * @param BoneName The name of the bone to map.
     * @return The mapped body region name, or NAME_None if no mapping exists.
     */
    FName MapBoneToRegion(const FName& BoneName) const;
};
