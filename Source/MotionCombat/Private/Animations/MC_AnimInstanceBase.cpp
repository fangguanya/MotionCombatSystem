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
 * MC_AnimInstanceBase.cpp
 * Base class for all Animation Instances in MotionCombat.
 * This class serves as a foundation for custom animation instances, providing a common interface and functionality.
 * Implements UAnimInstance and IIAnimation
*/

#include "Animations/MC_AnimInstanceBase.h"
#include "Kismet/KismetMathLibrary.h"

// Constructor
UMC_AnimInstanceBase::UMC_AnimInstanceBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUseMultiThreadedAnimationUpdate = true; // Enable multi-threaded animation updates
    RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly; // Set root motion mode to only from montages
}

// Called when the animation instance is destorying
void UMC_AnimInstanceBase::BeginDestroy()
{
    Super::BeginDestroy();

    try
    {
        CachedMovementComponent = nullptr; // Clear cached movement component pointer
        OwningPawn = nullptr; // Clear cached owning pawn pointer
        OwningCharacter = nullptr; // Clear the cached character pointer
    }
    catch (...)
    {
    }
}

// Initialize the animation instance
void UMC_AnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Initialize any variables or perform setup here
    if (APawn* Pawn = TryGetPawnOwner())
    {
        OwningPawn = Pawn; // Cache the owning pawn
        CachedMovementComponent = Cast<UCharacterMovementComponent>(OwningPawn->GetMovementComponent()); // Cache the movement component   
        OwningCharacter = Cast<AMC_CharacterBase>(OwningPawn); // Cache the owning character     
    }
}

// Called when the animation instance updates
void UMC_AnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
}

// Called when the animation instance updates thread safe
void UMC_AnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    try
    {
        // Perform any thread-safe updates here
        if (GetMovementComponent() && GetOwningPawn())
        {
            /**
             * Update IsInAir based on the movement component
            */
            IsInAir = GetMovementComponent()->IsFalling();

            /**
             * Update velocity.
             */
            Velocity = GetMovementComponent()->Velocity;

            /**
             * Update speed.
            */
            Speed = GetOwningPawn()->GetVelocity().Length();
            float CharacterMaxSpeed = GetMovementComponent()->GetMaxSpeed();
            Speed = FMath::Clamp(Speed, 0.0f, CharacterMaxSpeed); // Clamp Speed to a character maximum.

            /**
             * Update IsMoving based on current speed.
             * Works for both player and AI (including StateTree movement).
             */
            IsMoving = Speed > 2.0f; // small threshold prevents jitter at near-zero speed

            /**
             * Update Roll, Pitch, and Yaw based on the owning pawn's rotation.
             */

             // Get owning pawn's rotation & base aim rotation
            FRotator AimRotation = GetOwningPawn()->GetBaseAimRotation(); // This is the rotation the pawn is aiming at
            Rotation = GetOwningPawn()->GetActorRotation(); // This is the rotation of the pawn itself
            FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, Rotation); // Calculate the delta rotation between aim and actor rotation

            // Update Roll, Pitch, and Yaw based on the delta rotation
            Roll = DeltaRot.Roll;
            Pitch = DeltaRot.Pitch;
            Yaw = DeltaRot.Yaw;

            /**
             * Update Direction based on velocity and actor rotation.
             */
            Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
            SetEDirection();

            /**
             * Update Yaw Delta for leans
            */

            // Calculate the delta rotation between Rotation Last Tick and actor rotation
            FRotator DeltaRot2 = UKismetMathLibrary::NormalizedDeltaRotator(RotationLastTick, Rotation);

            const float leanIntensity = 7.0f; // Adjust this value to control the intensity of the lean
            const float InterpSpeed = 6.0f;
            const float DeltaTime = 0.0f;
            float CurrentYawDelta = DeltaRot2.Yaw; // your existing float value
            float TargetYawDelta = (DeltaRot.Yaw / 1.f) / leanIntensity;
            YawDelta = FMath::FInterpTo(CurrentYawDelta, TargetYawDelta, DeltaTime, InterpSpeed);

            /**
             * Update rotation last tick.
            */
            RotationLastTick = Rotation;

            // Update IsAccelerating based on the velocity.
            IsAccelerating = GetMovementComponent()->GetCurrentAcceleration().Size() > 2.0f;

            // Update full body flag based on the "FullBody" curve value
            FullBody = GetCurveValue(FName("FullBody")) > 0.0f;

            // Update IsCrouched based on the owning pawn's crouch state
            IsCrouched = GetOwningCharacter()->bIsCrouched;

            // ============================
            // Determine CharacterAnimState
            // ============================
            if (IsInAir) // In Air
            {
                CharacterAnimState = EMC_CharacterAnimState::InAir;
            }
            else if (IsBlocking)
            {
                if (Speed < 10.f)
                {
                    CharacterAnimState = EMC_CharacterAnimState::BlockingIdle;
                }
                else if (Speed < WalkMaxSpeed)
                {
                    CharacterAnimState = EMC_CharacterAnimState::BlockingWalking;
                }
                else if (Speed < JogMaxSpeed)
                {
                    CharacterAnimState = EMC_CharacterAnimState::BlockingJogging;
                }
                else
                {
                    CharacterAnimState = EMC_CharacterAnimState::BlockingRunning;
                }
            }
            else if (IsCrouched)
            {
                if (Speed < 10.f)
                {
                    CharacterAnimState = EMC_CharacterAnimState::CrouchedIdle;
                }
                else
                {
                    CharacterAnimState = EMC_CharacterAnimState::CrouchedWalking;
                }
            }
            else // Standing & not blocking
            {
                if (Speed < 10.f)
                {
                    CharacterAnimState = EMC_CharacterAnimState::StandingIdle;
                }
                else if (Speed < WalkMaxSpeed)
                {
                    CharacterAnimState = EMC_CharacterAnimState::Walking;
                }
                else if (Speed < JogMaxSpeed)
                {
                    CharacterAnimState = EMC_CharacterAnimState::Jogging;
                }
                else
                {
                    CharacterAnimState = EMC_CharacterAnimState::Running;
                }
            }
        }
    }
    catch (...)
    {
    }
}
