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
 * Date: 11-3-2025
 * =============================================================================
 * MCS_GlobalFunctions.h
 *
 * Description:
 *  Blueprint-accessible global utility library for the Motion Combat System.
 *  Provides static helper functions for commonly needed math, vector, and
 *  combat-related calculations that can be called anywhere in Blueprint or C++.
 *
 *  Example uses:
 *   - Determining if one actor is facing another.
 *   - Performing direction and angle checks.
 *   - Computing direction vectors for attacks or movement alignment.
 *   - Supporting AI perception, targeting, or animation conditions.
 *
 *  This library is stateless and lightweight, intended to simplify common
 *  geometric and spatial checks for combat behaviors.
 * =============================================================================
 */

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "MCS_GlobalFunctions.generated.h"

/**
 * UMCS_GlobalFunctions
 *
 * A collection of static math and direction utility functions used throughout
 * the Motion Combat System. These functions are designed for both Blueprint
 * and C++ accessibility, providing consistent geometric logic for facing,
 * distance, and orientation checks.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Motion Combat Global Functions"))
class MOTIONCOMBATSYSTEM_API UMCS_GlobalFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Checks whether one actor is facing another within a given tolerance angle.
     *
     * @param ActorA
     *      The actor whose facing direction will be evaluated.
     * @param ActorB
     *      The actor being checked if ActorA is facing it.
     * @param ToleranceDegrees
     *      The maximum allowed angle (in degrees) between ActorA's forward vector
     *      and the direction vector toward ActorB. Default = 45°.
     *
     * @return
     *      True if ActorA is facing ActorB within the specified tolerance angle.
     *      False if the angle exceeds tolerance or either actor is invalid.
     *
     * @note
     *      Uses a dot product and arccosine to determine the facing angle.
     *      Commonly used for directional hit checks, perception, and AI logic.
     */
    UFUNCTION(BlueprintPure, Category = "Motion Combat|Math", meta = (DisplayName = "Is Actor Facing Target"))
    static bool IsActorFacingTarget(const AActor* ActorA, const AActor* ActorB, float ToleranceDegrees = 45.f);

    /**
     * Computes the signed horizontal angle (in degrees) between two actors.
     *
     * @param ActorA
     *      Reference actor whose forward vector defines zero degrees.
     * @param ActorB
     *      Target actor to measure the angle toward.
     *
     * @return
     *      The signed angle between ActorA’s forward vector and the direction
     *      to ActorB. Positive values indicate ActorB is to the right of ActorA;
     *      negative values indicate ActorB is to the left.
     *
     * @note
     *      Useful for determining side-specific hit reactions or strafe logic.
     */
    UFUNCTION(BlueprintPure, Category = "Motion Combat|Math", meta = (DisplayName = "Get Angle Between Actors"))
    static float GetAngleBetweenActors(const AActor* ActorA, const AActor* ActorB);

    /**
     * Returns a normalized world-space direction vector pointing from one actor to another.
     *
     * @param FromActor
     *      The actor serving as the start point.
     * @param ToActor
     *      The actor serving as the target or destination.
     *
     * @return
     *      A normalized (unit length) direction vector from FromActor to ToActor.
     *      Returns ZeroVector if either actor is invalid.
     */
    UFUNCTION(BlueprintPure, Category = "Motion Combat|Math", meta = (DisplayName = "Get Direction To Target"))
    static FVector GetDirectionToTarget(const AActor* FromActor, const AActor* ToActor);

    /**
     * Returns the planar (2D) distance between two actors, ignoring vertical (Z) height.
     *
     * @param ActorA
     *      The first actor.
     * @param ActorB
     *      The second actor.
     *
     * @return
     *      The horizontal distance (in world units) between ActorA and ActorB.
     *
     * @note
     *      Commonly used for grounded movement or navigation distance checks.
     */
    UFUNCTION(BlueprintPure, Category = "Motion Combat|Math", meta = (DisplayName = "Get 2D Distance Between Actors"))
    static float GetDistance2D(const AActor* ActorA, const AActor* ActorB);
};
