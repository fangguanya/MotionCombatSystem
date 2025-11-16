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
 * MCS_GlobalFunctions.cpp
 *
 * Description:
 *  Implements global math and spatial helper functions for the Motion Combat System.
 *  Provides facing, direction, and distance utilities commonly used in targeting,
 *  attack selection, and perception logic.
 * =============================================================================
 */

#include "Libraries/MCS_GlobalFunctions.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"


/**
 * Determines if ActorA is facing ActorB within a given tolerance angle (degrees).
 */
bool UMCS_GlobalFunctions::IsActorFacingTarget(const AActor* ActorA, const AActor* ActorB, float ToleranceDegrees)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
        return false;

    // ActorA’s forward vector and normalized direction to ActorB
    const FVector Forward = ActorA->GetActorForwardVector();
    const FVector ToTarget = (ActorB->GetActorLocation() - ActorA->GetActorLocation()).GetSafeNormal();

    // Compute facing angle
    const float Dot = FVector::DotProduct(Forward, ToTarget);
    const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

    return Angle <= ToleranceDegrees;
}

/**
 * Returns the signed horizontal angle between ActorA’s forward vector and the direction toward ActorB.
 */
float UMCS_GlobalFunctions::GetAngleBetweenActors(const AActor* ActorA, const AActor* ActorB)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
        return 0.f;

    const FVector Forward = ActorA->GetActorForwardVector();
    const FVector ToTarget = (ActorB->GetActorLocation() - ActorA->GetActorLocation()).GetSafeNormal();

    const float Dot = FVector::DotProduct(Forward, ToTarget);
    const float CrossZ = FVector::CrossProduct(Forward, ToTarget).Z;

    // atan2 returns signed angle (-180° to 180°)
    return FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
}

/**
 * Returns a normalized direction vector from one actor to another.
 */
FVector UMCS_GlobalFunctions::GetDirectionToTarget(const AActor* FromActor, const AActor* ToActor)
{
    if (!FromActor || !ToActor)
        return FVector::ZeroVector;

    return (ToActor->GetActorLocation() - FromActor->GetActorLocation()).GetSafeNormal();
}

/**
 * Returns planar (XY-only) distance between two actors, ignoring height.
 */
float UMCS_GlobalFunctions::GetDistance2D(const AActor* ActorA, const AActor* ActorB)
{
    if (!IsValid(ActorA) || !IsValid(ActorB))
        return 0.f;

    return FVector::Dist2D(ActorA->GetActorLocation(), ActorB->GetActorLocation());
}
