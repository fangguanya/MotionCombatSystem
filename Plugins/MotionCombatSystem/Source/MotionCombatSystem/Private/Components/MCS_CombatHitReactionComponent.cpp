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
 * MCS_CombatHitReactionComponent.cpp
 *
 * Implementation for the combat hit reaction component.
 * Handles direction + severity-based hit reactions via DataTable-driven montages.
 */

#include <Components/MCS_CombatHitReactionComponent.h>
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"


 // Constructor
UMCS_CombatHitReactionComponent::UMCS_CombatHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMCS_CombatHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();
}

/**
 * Performs a hit reaction using hit result data.
 * Automatically determines direction from the hit location and selects
 * a matching reaction montage based on bone, region, direction, and severity.
 *
 * @param Hit - The hit result data containing impact point and bone info.
 * @param TargetActor - The actor that was hit (usually GetOwner()).
 * @param Severity - The hit severity (Light, Heavy, Knockback, etc.).
 */
void UMCS_CombatHitReactionComponent::PerformHitReaction(const FHitResult& Hit, AActor* TargetActor, EPGAS_HitSeverity Severity)
{
    if (!HitReactionDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] No HitReactionDataTable assigned."));
        return;
    }

    if (!IsValid(TargetActor))
    {
        TargetActor = GetOwner();
        if (!IsValid(TargetActor))
        {
            UE_LOG(LogTemp, Warning, TEXT("[HitReaction] Invalid TargetActor."));
            return;
        }
    }

    const FVector HitLocation = Hit.ImpactPoint;
    const FName BoneName = Hit.BoneName;

    // Determine hit direction relative to actor
    const EMCS_Direction Direction = CalculateHitDirection(HitLocation, TargetActor);

    // Look up reaction from DataTable
    const FMCS_HitReaction* Reaction = FindReaction(BoneName, Direction, Severity);
    if (!Reaction)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] No matching reaction found (Bone=%s, Dir=%d, Severity=%d)."),
            *BoneName.ToString(), static_cast<int32>(Direction), static_cast<int32>(Severity));
        return;
    }

    if (!Reaction->Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] Reaction montage is null (Bone=%s, Region=%s)."),
            *Reaction->TargetBone.ToString(), *Reaction->TargetRegion.ToString());
        return;
    }

    // Play montage
    PlayMontageInternal(Reaction->Montage, Reaction->PlayRate);

    UE_LOG(LogTemp, Log, TEXT("[HitReaction] Playing reaction: %s (Bone=%s, Region=%s, Dir=%d, Severity=%d)"),
        *Reaction->Montage->GetName(),
        *Reaction->TargetBone.ToString(),
        *Reaction->TargetRegion.ToString(),
        static_cast<int32>(Direction),
        static_cast<int32>(Severity));
}


/**
 * Finds the best matching hit reaction using bone, region, direction, and severity hierarchy.
 * @param BoneName - The bone struck (can be NAME_None).
 * @param Direction - The direction of the hit (Forward, Back, etc.).
 * @param Severity - The severity of the hit (Light, Heavy, Knockdown, etc.).
 * @return The best matching hit reaction, or nullptr if none found.
 */
const FMCS_HitReaction* UMCS_CombatHitReactionComponent::FindReaction(const FName& BoneName, EMCS_Direction Direction, EPGAS_HitSeverity Severity) const
{
    if (!HitReactionDataTable) return nullptr;

    static const FString Context(TEXT("FindReaction"));
    TArray<FMCS_HitReaction*> AllRows;
    HitReactionDataTable->GetAllRows(Context, AllRows);

    const FMCS_HitReaction* ExactBoneMatch = nullptr;
    const FMCS_HitReaction* RegionMatch = nullptr;
    const FMCS_HitReaction* DirectionMatch = nullptr;
    const FMCS_HitReaction* SeverityOnly = nullptr;

    // Map bone to high-level region name
    const FName MappedRegion = MapBoneToRegion(BoneName);

    for (const FMCS_HitReaction* Row : AllRows)
    {
        if (!Row) continue;

        const bool bSeverityMatch = (Row->Severity == Severity);
        const bool bDirectionMatch = (Row->Direction == Direction || Row->Direction == EMCS_Direction::None);

        // Bone-specific match (highest priority)
        if (bSeverityMatch && Row->TargetBone == BoneName && !BoneName.IsNone())
        {
            ExactBoneMatch = Row;
            break; // stop immediately — best possible match
        }

        // Region match (second priority)
        if (bSeverityMatch && !MappedRegion.IsNone() && Row->TargetRegion == MappedRegion)
        {
            RegionMatch = Row;
            // don’t break; might still find exact bone later
        }

        // Direction + Severity match (third priority)
        if (bSeverityMatch && bDirectionMatch && Row->TargetBone.IsNone() && Row->TargetRegion.IsNone())
        {
            DirectionMatch = Row;
        }

        // Severity-only fallback (lowest priority)
        if (bSeverityMatch && Row->Direction == EMCS_Direction::None &&
            Row->TargetBone.IsNone() && Row->TargetRegion.IsNone())
        {
            SeverityOnly = Row;
        }
    }

    // Return in hierarchical order of specificity
    if (ExactBoneMatch) return ExactBoneMatch;
    if (RegionMatch) return RegionMatch;
    if (DirectionMatch) return DirectionMatch;
    if (SeverityOnly) return SeverityOnly;

    return nullptr;
}

/**
 * Helper: Plays a montage on the owning actor's mesh if valid
 * @param Montage The montage to play
 * @param InPlayRate The play rate for the montage
 */
void UMCS_CombatHitReactionComponent::PlayMontageInternal(UAnimMontage* Montage, float InPlayRate)
{
    if (!Montage)
        return;

    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!IsValid(CharacterOwner) || !IsValid(CharacterOwner->GetMesh()))
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] Invalid Character or Mesh."));
        return;
    }

    UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HitReaction] Missing AnimInstance."));
        return;
    }

    // Stop any current reaction to ensure clarity
    AnimInstance->Montage_Stop(0.1f);
    AnimInstance->Montage_Play(Montage, InPlayRate);
}

/**
 * Calculates hit direction based on hit location relative to the target actor.
 * @param HitLocation The world location of the hit.
 * @param TargetActor The actor that was hit.
 * @return The calculated hit direction enum value.
 */
EMCS_Direction UMCS_CombatHitReactionComponent::CalculateHitDirection(const FVector& HitLocation, AActor* TargetActor)
{
    if (!IsValid(TargetActor))
        return EMCS_Direction::None;

    const FVector ActorLocation = TargetActor->GetActorLocation();
    const FVector Forward = TargetActor->GetActorForwardVector();
    const FVector Right = TargetActor->GetActorRightVector();

    // Direction from actor to hit
    const FVector ToHit = (HitLocation - ActorLocation).GetSafeNormal();

    // Dot products for angle comparison
    const float ForwardDot = FVector::DotProduct(Forward, ToHit);
    const float RightDot = FVector::DotProduct(Right, ToHit);

    // Determine primary direction quadrant
    // Forward/back is dominant axis (based on sign of ForwardDot)
    // Right/left is secondary
    if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
    {
        return (ForwardDot >= 0.0f) ? EMCS_Direction::Forward : EMCS_Direction::Back;
    }
    else
    {
        return (RightDot >= 0.0f) ? EMCS_Direction::Right : EMCS_Direction::Left;
    }
}

/**
 * Internal helper that maps bone names to simplified body regions (e.g., "LegLeft", "Torso").
 * @param BoneName The name of the bone to map.
 * @return The mapped body region name, or NAME_None if no mapping exists.
 */
FName UMCS_CombatHitReactionComponent::MapBoneToRegion(const FName& BoneName) const
{
    if (BoneName.IsNone())
        return NAME_None;

    const FString Bone = BoneName.ToString().ToLower();

    // ---- HEAD ----
    if (Bone.Contains(TEXT("head")) || Bone.Contains(TEXT("neck")))
    {
        return FName("Head");
    }

    // ---- TORSO / SPINE ----
    if (Bone.Contains(TEXT("spine")) || Bone.Contains(TEXT("pelvis")) || Bone.Contains(TEXT("root")))
    {
        return FName("Torso");
    }

    // ---- LEFT ARM ----
    if (Bone.Contains(TEXT("upperarm_l")) || Bone.Contains(TEXT("lowerarm_l")) || Bone.Contains(TEXT("hand_l")) || Bone.Contains(TEXT("shoulder_l")))
    {
        return FName("ArmLeft");
    }

    // ---- RIGHT ARM ----
    if (Bone.Contains(TEXT("upperarm_r")) || Bone.Contains(TEXT("lowerarm_r")) || Bone.Contains(TEXT("hand_r")) || Bone.Contains(TEXT("shoulder_r")))
    {
        return FName("ArmRight");
    }

    // ---- LEFT LEG ----
    if (Bone.Contains(TEXT("thigh_l")) || Bone.Contains(TEXT("calf_l")) || Bone.Contains(TEXT("foot_l")) || Bone.Contains(TEXT("ball_l")))
    {
        return FName("LegLeft");
    }

    // ---- RIGHT LEG ----
    if (Bone.Contains(TEXT("thigh_r")) || Bone.Contains(TEXT("calf_r")) || Bone.Contains(TEXT("foot_r")) || Bone.Contains(TEXT("ball_r")))
    {
        return FName("LegRight");
    }

    // ---- Default / Unknown ----
    return NAME_None;
}
