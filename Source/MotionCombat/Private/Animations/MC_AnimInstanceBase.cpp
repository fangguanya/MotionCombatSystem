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
    CachedMovementComponent = nullptr; // Clear cached movement component pointer
    OwningPawn = nullptr; // Clear cached owning pawn pointer
    OwningCharacter = nullptr; // Clear the cached character pointer

    Super::BeginDestroy();
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

/**
 * Called when the animation instance updates on the game thread
 *@param DeltaSeconds - The time since the last update
 * This function is called on the game thread and is safe to access actors and components.
 */
void UMC_AnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid(OwningPawn) || !IsValid(CachedMovementComponent) || !IsValid(OwningCharacter))
    {
        return;
    }

    /*
     *Gather ALL game-thread values
     */

     // Movement
    IsInAir = CachedMovementComponent->IsFalling();
    Velocity = CachedMovementComponent->Velocity;

    // Speed
    Speed = Velocity.Length();
    Speed = FMath::Clamp(Speed, 0.0f, CachedMovementComponent->GetMaxSpeed());
    
    // Rotations
    FRotator AimRotation = OwningPawn->GetBaseAimRotation();
    Rotation = OwningPawn->GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, Rotation);

    Roll = DeltaRot.Roll;
    Pitch = DeltaRot.Pitch;
    Yaw = DeltaRot.Yaw;

    // Direction
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
    SetEDirection();

    // Leaning
    FRotator DeltaRot2 = UKismetMathLibrary::NormalizedDeltaRotator(RotationLastTick, Rotation);
    RotationLastTick = Rotation; // Store for next frame

    const float leanIntensity = 7.0f;
    const float InterpSpeed = 6.0f;
    float CurrentYawDelta = DeltaRot2.Yaw;
    float TargetYawDelta = (DeltaRot.Yaw / 1.f) / leanIntensity;
    YawDelta = FMath::FInterpTo(CurrentYawDelta, TargetYawDelta, DeltaSeconds, InterpSpeed);

    // Acceleration
    IsAccelerating = CachedMovementComponent->GetCurrentAcceleration().Size() > 2.0f;

    // Crouch
    IsCrouched = OwningCharacter->bIsCrouched;
}

/**
 * Called when the animation instance updates on a worker thread
 * @param DeltaSeconds - The time since the last update
 * This function is called on a worker thread and should not access actors or components.
 */
void UMC_AnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    // You may optionally do work here using only:
    // - math
    // - stored anim instance variables
    // - curve values
    // - pure animation data

    // Determine if the character is moving
    IsMoving = Speed > 2.0f;

    // Curves (Safe)
    FullBody = GetCurveValue(FName("FullBody")) > 0.0f;

    // Determine Character Anim State
    if (IsInAir)
    {
        CharacterAnimState = EMC_CharacterAnimState::InAir;
    }
    else if (IsBlocking)
    {
        if (Speed < 10.f)
            CharacterAnimState = EMC_CharacterAnimState::BlockingIdle;
        else if (Speed < WalkMaxSpeed)
            CharacterAnimState = EMC_CharacterAnimState::BlockingWalking;
        else if (Speed < JogMaxSpeed)
            CharacterAnimState = EMC_CharacterAnimState::BlockingJogging;
        else
            CharacterAnimState = EMC_CharacterAnimState::BlockingRunning;
    }
    else if (IsCrouched)
    {
        CharacterAnimState = (Speed < 10.f)
            ? EMC_CharacterAnimState::CrouchedIdle
            : EMC_CharacterAnimState::CrouchedWalking;
    }
    else
    {
        if (Speed < 10.f)
            CharacterAnimState = EMC_CharacterAnimState::StandingIdle;
        else if (Speed < WalkMaxSpeed)
            CharacterAnimState = EMC_CharacterAnimState::Walking;
        else if (Speed < JogMaxSpeed)
            CharacterAnimState = EMC_CharacterAnimState::Jogging;
        else
            CharacterAnimState = EMC_CharacterAnimState::Running;
    }
}
