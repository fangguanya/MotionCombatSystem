/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat Sample
 * This is a sample project showcasing the Motion Combat System plugin for Unreal Engine.
 * Author: Christopher D. Parker
 * Date: 11-10-2025
 * =============================================================================
 * MC_AnimInstanceBase.h
 * Base class for all Animation Instances in MotionCombat.
 * This class serves as a foundation for custom animation instances, providing a common interface and functionality.
 * Implements UAnimInstance and IIAnimation
*/

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Enums/MC_Direction.h>
#include <Enums/MC_CharacterAnimState.h>
#include <Characters/MC_CharacterBase.h>
#include "MC_AnimInstanceBase.generated.h"


/**
 * Base class for all Animation Instances in MotionCombat
 */
UCLASS(meta = (DisplayName = "MC Anim Instance Base", ShortTooltip = "Base class for all Animation Instances in MotionCombat"))
class MOTIONCOMBAT_API UMC_AnimInstanceBase : public UAnimInstance
{
    GENERATED_BODY()

public:
    /*
     * Functions
    */

    // Constructor
    UMC_AnimInstanceBase(const FObjectInitializer& ObjectInitializer);

    // Returns the movement component of the character, thread-safe
    // This function is thread-safe and can be called from any thread.
    UFUNCTION(BlueprintPure, Category = "Animation", meta = (BlueprintThreadSafe))
    const UCharacterMovementComponent* GetMovementComponent() const { return CachedMovementComponent.Get(); }

    // Returns the owning pawn of the animation instance, thread-safe
    // This function is thread-safe and can be called from any thread.
    UFUNCTION(BlueprintPure, Category = "Animation", meta = (BlueprintThreadSafe))
    APawn* GetOwningPawn() { return OwningPawn.Get(); }

    // Returns the owning character of the animation instance, thread-safe
    // This function is thread-safe and can be called from any thread.
    UFUNCTION(BlueprintPure, Category = "Animation", meta = (BlueprintThreadSafe))
    AMC_CharacterBase* GetOwningCharacter() { return OwningCharacter.Get(); }

    /*
     * Properties
    */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Roll;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Yaw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float YawDelta;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FRotator RotationLastTick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool FullBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsBlocking;

    // Whether the player is currently locked on to a target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsTargetLocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float Direction = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EMC_Direction eDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsCrouched;

    // Current high-level character animation state (Mostly used for Motion Matching and Chooser Tables)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    EMC_CharacterAnimState CharacterAnimState = EMC_CharacterAnimState::StandingIdle;

    /** Maximum speed (cm/s) considered as walking (Jog starts after this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Speed Thresholds",
        meta = (ClampMin = "0.0", ClampMax = "600.0", Tooltip = "Maximum speed (cm/s) considered as walking (Jog starts after this)", DisplayName = "Walk Max Speed"))
    float WalkMaxSpeed = 180.0f;

    /** Maximum speed (cm/s) considered as jogging (Run starts after this) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Speed Thresholds",
        meta = (ClampMin = "0.0", ClampMax = "600.0", Tooltip = "Maximum speed (cm/s) considered as jogging (Run starts after this)", DisplayName = "Jog Max Speed"))
    float JogMaxSpeed = 400.0f;

protected:
    /*
     * Functions
    */

    // Called before destroying the object.
    virtual void BeginDestroy() override;

    /** Called when the animation instance is initialized */
    virtual void NativeInitializeAnimation() override;

    // Called every frame
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // Native thread-safe update for any work not relying on game thread
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // Function to set eDirection based on Direction float value.
    void SetEDirection()
    {
        // Normalize the angle to [-180, 180]
        float NormalizedDir = FRotator::ClampAxis(Direction); // Clamp to [0,360)
        if (NormalizedDir > 180.f)
        {
            NormalizedDir -= 360.f; // Convert to [-180,180]
        }

        if (NormalizedDir >= -45.f && NormalizedDir <= 45.f)
        {
            eDirection = EMC_Direction::Forward;
        }
        else if (NormalizedDir > 45.f && NormalizedDir < 135.f)
        {
            eDirection = EMC_Direction::Right;
        }
        else if (NormalizedDir >= 135.f || NormalizedDir <= -135.f)
        {
            eDirection = EMC_Direction::Backward;
        }
        else // (-135, -45)
        {
            eDirection = EMC_Direction::Left;
        }
    }

private:
    /*
     * Properties
    */

    // Cached movement component pointer (game thread only)
    TObjectPtr<const UCharacterMovementComponent> CachedMovementComponent;

    // Cached pawn pointer (game thread only)
    TObjectPtr<APawn> OwningPawn;

    // Cached player character pointer (game thread only)
    UPROPERTY()
    TObjectPtr<AMC_CharacterBase> OwningCharacter;

    // Cached actor rotation (game thread only)
    FRotator OwningPawnActorRotation = FRotator::ZeroRotator;
};
