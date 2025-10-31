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
 * Date: 10-14-2025
 * =============================================================================
 * MCS_CombatCoreComponent.cpp
 * Implements the logic for loading, choosing, and executing attacks
 * using the targeting subsystem and data tables.
 */

#include <Components/MCS_CombatCoreComponent.h>
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h" 
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

 // Local dependency: used only for pulling defensive state info
#include <Components/MCS_CombatDefenseComponent.h>

#if WITH_EDITORONLY_DATA
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Engine.h"
#endif


// Constructor
UMCS_CombatCoreComponent::UMCS_CombatCoreComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMCS_CombatCoreComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwnerActor())
    {
        // Cache hitbox component reference
        CachedHitboxComp = GetOwnerActor()->FindComponentByClass<UMCS_CombatHitboxComponent>();
    }

    // Get the targeting subsystem from world
    if (UWorld* World = GetWorld())
    {
        TargetingSubsystem = World->GetSubsystem<UMCS_TargetingSubsystem>();
    }

    // If no active set defined but map has entries, activate the first
    if (!ActiveAttackSetTag.IsValid() && AttackSets.Num() > 0)
    {
        const FGameplayTag FirstKey = AttackSets.CreateConstIterator()->Key;
        SetActiveAttackSet(FirstKey);
    }

    // Bind to targeting updates
    if (TargetingSubsystem)
    {
        TargetingSubsystem->OnTargetsUpdated.AddDynamic(this, &UMCS_CombatCoreComponent::HandleTargetsUpdated);
    }
}

// Called when the game ends
void UMCS_CombatCoreComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unbind from targeting updates
    if (TargetingSubsystem)
    {
        TargetingSubsystem->OnTargetsUpdated.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleTargetsUpdated);
    }

    // Unbind all notifies
    UnbindAllNotifies();

    // Clear chooser pool
    ClearChooserPool();

    Super::EndPlay(EndPlayReason);
}

void UMCS_CombatCoreComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    // UpdatePlayerSituation(DeltaTime); // Uncomment if you want to update PlayerSituation every frame in C++. Can be done in Blueprint instead.
}

/*
 * Plays the selected attack's montage if valid
 * @param DesiredType - type of attack to perform
 * @param DesiredDirection - direction of the attack in world space
*/
void UMCS_CombatCoreComponent::PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    if (!SelectAttack(DesiredType, DesiredDirection, CurrentSituation))
    {
        return;
    }

    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner || !CurrentAttack.HasValidMontage()) return;

    // Cache hitbox component reference
    CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();

    // Bind notifies for the montage
    BindNotifiesForMontage(CurrentAttack.AttackMontage);

    // Retrieve anim instance
    UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        return;
    }

    //----------------------------------------
    // Smoothly blend between montages
    //----------------------------------------

    // Use designer-defined or default blend times
    float BlendInTime = FMath::Max(CurrentAttack.BlendInTime, 0.0f);
    float BlendOutTime = FMath::Max(CurrentAttack.BlendOutTime, 0.0f);

    // Use faster blend when chaining combos
    const bool bFromCombo = bIsComboWindowOpen;
    if (bFromCombo)
    {
        BlendInTime = FMath::Min(BlendInTime, 0.05f);
        BlendOutTime = FMath::Min(BlendOutTime, 0.05f);
    }

    // Smoothly fade out any active montage
    if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
    {
        if (CurrentMontage != CurrentAttack.AttackMontage)
        {
            AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
        }
    }

    // Apply blend parameters to the new montage
    if (CurrentAttack.AttackMontage)
    {
        CurrentAttack.AttackMontage->BlendIn.SetBlendTime(BlendInTime);
        CurrentAttack.AttackMontage->BlendOut.SetBlendTime(BlendOutTime);
    }

    // Play the new montage with blending
    const float PlayRate = 1.0f;
    const float StartTime = 0.0f;
    AnimInstance->Montage_Play(CurrentAttack.AttackMontage, PlayRate, EMontagePlayReturnType::MontageLength, StartTime, true);

    // Broadcast attack started event to event bus
    if (UWorld* World = GetWorld())
    {
        if (UMCS_CombatEventBus* Bus = UMCS_CombatEventBus::Get(World))
        {
            AActor* Target = GetClosestTarget(2500.f);
            Bus->OnAttackStarted.Broadcast(GetOwner(), Target);
        }
    }

    // Jump to specified section if provided
    if (CurrentAttack.MontageSection != NAME_None)
    {
        AnimInstance->Montage_JumpToSection(CurrentAttack.MontageSection, CurrentAttack.AttackMontage);
    }
}

/*
 * Chooses an appropriate attack using AttackChooser and available targets
 * @param DesiredType - type of attack to select
 * @param DesiredDirection - direction of the attack in world space
*/
bool UMCS_CombatCoreComponent::SelectAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser)
    {
        return false;
    }

    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor)
    {
        return false;
    }

    //----------------------------------------
    // 1. Gather filtered entries by DesiredType
    //----------------------------------------
    TArray<FMCS_AttackEntry> FilteredEntries;

    if (ActiveSet->AttackDataTable)
    {
        TArray<FMCS_AttackEntry*> Rows;
        ActiveSet->AttackDataTable->GetAllRows(TEXT("SelectAttack"), Rows);

        for (const FMCS_AttackEntry* Row : Rows)
        {
            if (Row && Row->AttackType == DesiredType)
            {
                FilteredEntries.Add(*Row);
            }
        }
    }

    if (FilteredEntries.IsEmpty())
    {
        return false;
    }

    //----------------------------------------
    // 2. Get a chooser instance from the pool. (Memory optimization)
    //----------------------------------------
    UMCS_AttackChooser* TempChooser = IsValid(ActiveAttackChooser.Get())
        ? ActiveAttackChooser.Get()                                     // raw pointer
        : NewObject<UMCS_AttackChooser>(this, ActiveSet->AttackChooser);

    if (!IsValid(TempChooser))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Failed to create or retrieve AttackChooser instance."));
        return false;
    }

    // Refresh entries
    TempChooser->AttackEntries = FilteredEntries;

    //----------------------------------------
    // 3. Gather valid targets
    //----------------------------------------
    TArray<AActor*> Targets;
    if (TargetingSubsystem)
    {
        for (const FMCS_TargetInfo& Info : TargetingSubsystem->GetAllTargets())
        {
            if (IsValid(Info.TargetActor))
            {
                Targets.Add(Info.TargetActor);
            }
        }
    }

    //----------------------------------------
    // 4. Cache situation
    //----------------------------------------
    PlayerSituation = CurrentSituation;

    //----------------------------------------
    // 5. Choose the best attack
    //----------------------------------------
    FMCS_AttackEntry ChosenAttack;
    const bool bSuccess = TempChooser->ChooseAttack(OwnerActor, Targets, DesiredDirection, CurrentSituation, ChosenAttack);

    if (bSuccess)
    {
        CurrentAttack = ChosenAttack;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No valid attack chosen for type: %s"),
            *UEnum::GetValueAsString(DesiredType));
    }

    return bSuccess;
}

/*
 * Attempts to chain into the next attack in a combo
 * @param DesiredType - type of attack to select
 * @param DesiredDirection - direction of the attack in world space
 */
bool UMCS_CombatCoreComponent::TryContinueCombo(
    EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation)
{
    if (!bIsComboWindowOpen)
    {
        return false;
    }

    if (AllowedComboNames.IsEmpty())
    {
        return false;
    }

    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser) return false;

    UMCS_AttackChooser* Chooser = IsValid(ActiveAttackChooser.Get())
        ? ActiveAttackChooser.Get()
        : NewObject<UMCS_AttackChooser>(this, ActiveSet->AttackChooser);
    
    AActor* OwnerActor = GetOwnerActor();
    if (!OwnerActor) return false;

    // Filter by allowed combo names
    TArray<FMCS_AttackEntry> Filtered;
    for (const FMCS_AttackEntry& Entry : Chooser->AttackEntries)
    {
        if (AllowedComboNames.Contains(Entry.AttackName))
        {
            Filtered.Add(Entry);
        }
    }

    if (Filtered.IsEmpty())
    {
        return false;
    }

    // Temporarily override chooser entries
    const TArray<FMCS_AttackEntry> Original = Chooser->AttackEntries;
    Chooser->AttackEntries = Filtered;

    FMCS_AttackEntry NextAttack;
    bool bChosen = Chooser->ChooseAttack(OwnerActor, {}, DesiredDirection, CurrentSituation, NextAttack);

    Chooser->AttackEntries = Original;

    if (!bChosen)
    {
        return false;
    }

    // Chain into next attack
    CurrentAttack = NextAttack;
    PerformAttack(DesiredType, DesiredDirection, CurrentSituation);

    // Reset combo window state (will be reopened by next montage’s combo notify)
    bCanContinueCombo = false;
    bIsComboWindowOpen = false;
    AllowedComboNames.Reset();

    return true;
}

/*
 * Gets the closest valid target via TargetingSubsystem
 */
AActor* UMCS_CombatCoreComponent::GetClosestTarget(float MaxRange) const
{
    if (!TargetingSubsystem)
        return nullptr;

    if (AActor* OwnerActor = GetOwnerActor())
    {
        return TargetingSubsystem->GetClosestTarget(OwnerActor->GetActorLocation(), MaxRange);
    }

    return nullptr;
}

/*
 * Utility to get the owning actor safely
 */
AActor* UMCS_CombatCoreComponent::GetOwnerActor() const
{
    return GetOwner();
}

// Handler for TargetingSubsystem target updates
void UMCS_CombatCoreComponent::HandleTargetsUpdated(const TArray<FMCS_TargetInfo>& NewTargets, int32 NewTargetCount)
{
    // UE_LOG(LogTemp, Log, TEXT("[CombatCore] Target list changed: %d targets in range."), NewTargetCount);

    // Fire the exposed Blueprint event
    if (OnTargetingUpdated.IsBound())
    {
        OnTargetingUpdated.Broadcast(NewTargets, NewTargetCount);
    }
}

/**
 * Utility to convert 2D movement input into an EMCS_AttackDirection enum value
 * @param MoveInput - 2D movement input vector (X=Forward/Backward, Y=Left/Right)
 * @return Corresponding EMCS_AttackDirection value
 */
EMCS_AttackDirection UMCS_CombatCoreComponent::GetAttackDirection(const FVector2D& MoveInput) const
{
    // If no significant input, treat as Omni (neutral)
    if (MoveInput.IsNearlyZero(0.2f))
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get the controlling actor (usually the player character)
    const AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get control rotation (camera-facing)
    FRotator ControlRot = OwnerActor->GetInstigatorController()
        ? OwnerActor->GetInstigatorController()->GetControlRotation()
        : FRotator::ZeroRotator;

    // Zero out pitch/roll — we only care about yaw
    ControlRot.Pitch = 0.f;
    ControlRot.Roll = 0.f;

    const FVector CameraForward = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X);
    const FVector CameraRight = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);

    // Convert 2D input into a world-space direction
    FVector DesiredDirectionWS = (CameraForward * MoveInput.Y + CameraRight * MoveInput.X).GetSafeNormal();

    if (DesiredDirectionWS.IsNearlyZero())
    {
        return EMCS_AttackDirection::Omni;
    }

    // Get forward vector of actor (used to compare relative direction)
    const FVector ActorForward = OwnerActor->GetActorForwardVector();
    const FVector ActorRight = OwnerActor->GetActorRightVector();

    // Calculate dot products
    const float ForwardDot = FVector::DotProduct(ActorForward, DesiredDirectionWS);
    const float RightDot = FVector::DotProduct(ActorRight, DesiredDirectionWS);

    // Determine facing quadrant using dot thresholds
    const float ForwardThreshold = 0.5f;  // cosine ~60 degrees
    const float SideThreshold = 0.5f;

    if (ForwardDot > ForwardThreshold)
    {
        return EMCS_AttackDirection::Forward;
    }
    else if (ForwardDot < -ForwardThreshold)
    {
        return EMCS_AttackDirection::Backward;
    }
    else if (RightDot > SideThreshold)
    {
        return EMCS_AttackDirection::Right;
    }
    else if (RightDot < -SideThreshold)
    {
        return EMCS_AttackDirection::Left;
    }

    // Fallback if between zones
    return EMCS_AttackDirection::Omni;
}

void UMCS_CombatCoreComponent::BindNotifiesForMontage(UAnimMontage* Montage)
{
    UnbindAllNotifies();
    if (!Montage) return;

    for (const FAnimNotifyEvent& Event : Montage->Notifies)
    {
        if (Event.NotifyStateClass && Event.NotifyStateClass->GetClass()->IsChildOf(UAnimNotifyState_MCSWindow::StaticClass()))
        {
            UAnimNotifyState_MCSWindow* Notify = Cast<UAnimNotifyState_MCSWindow>(Event.NotifyStateClass);
            if (Notify == nullptr) continue;

            // Bind to notify events. The type will be determined in the handler.
            Notify->OnNotifyBegin.AddDynamic(this, &UMCS_CombatCoreComponent::HandleMCSNotifyBegin);
            Notify->OnNotifyEnd.AddDynamic(this, &UMCS_CombatCoreComponent::HandleMCSNotifyEnd);

            BoundMCSNotifies.Add(Notify); // Keep track of bound notifies
        }
    }
}

void UMCS_CombatCoreComponent::UnbindAllNotifies()
{
    if (BoundMCSNotifies.Num() == 0) return;

    // Unbind all hitbox notify events
    for (UAnimNotifyState_MCSWindow* NotifyCDO1 : BoundMCSNotifies)
    {
        if (NotifyCDO1)
        {
            NotifyCDO1->OnNotifyBegin.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleMCSNotifyBegin);
            NotifyCDO1->OnNotifyEnd.RemoveDynamic(this, &UMCS_CombatCoreComponent::HandleMCSNotifyEnd);
        }
    }

    // Clear the lists
    BoundMCSNotifies.Reset();
}

void UMCS_CombatCoreComponent::HandleMCSNotifyBegin(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* Notify)
{
    // Validate the notify instance
    if (!Notify) return;

    // Validate owner
    const AActor* Owner = GetOwner();
    if (!Owner) return;

    // 🛡️ Guard: only run if this character is actively playing this montage
    if (const ACharacter* C = Cast<ACharacter>(Owner);
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage)))
        return;

    switch (EventType)
    {
        case EMCS_AnimEventType::HitboxWindow:
            // Get and/or cache the hitbox component
            if (!CachedHitboxComp)
            {
                if (ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
                    CachedHitboxComp = CharacterOwner->FindComponentByClass<UMCS_CombatHitboxComponent>();
            }
            if (!CachedHitboxComp) return;

            // Automatically reset hit tracking whenever a new hitbox window begins
            CachedHitboxComp->ResetAlreadyHit();

            // Start hit detection for this hitbox
            CachedHitboxComp->StartHitDetection(CurrentAttack, Notify->Hitbox);

            break;

        case EMCS_AnimEventType::ComboWindow:
            // Mark combo window as active
            bIsComboWindowOpen = true;

            // Load the allowed next attacks for this attack
            AllowedComboNames = CurrentAttack.AllowedNextAttacks;
            bCanContinueCombo = AllowedComboNames.Num() > 0;

            // Fire the combo begin event
            OnComboWindowBegin.Broadcast();

            break;

        case EMCS_AnimEventType::ParryWindow:
            OnParryWindowBegin.Broadcast(const_cast<AActor*>(Owner));
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Parry Window Begin for %s"), *Owner->GetName());

            // Broadcast to event bus
            if (UWorld* World = GetWorld())
            {
                if (UMCS_CombatEventBus* Bus = UMCS_CombatEventBus::Get(World))
                {
                    Bus->OnParryWindowOpened.Broadcast(const_cast<AActor*>(Owner), Notify->WindowLength);
                }
            }
            break;

        case EMCS_AnimEventType::DefenseWindow:
            OnDefenseWindowBegin.Broadcast(const_cast<AActor*>(Owner));
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Defense Window Begin for %s"), *Owner->GetName());

            // if (UWorld* World = GetWorld())
            // {
            //     if (UMCS_CombatEventBus* Bus = UMCS_CombatEventBus::Get(World))
            //     {
            //         // You can use duration or any additional data later
            //         Bus->OnBlockSuccess.Broadcast(const_cast<AActor*>(Owner), nullptr);
            //     }
            // }
            break;
    }
}

void UMCS_CombatCoreComponent::HandleMCSNotifyEnd(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* Notify)
{
    // Validate the notify instance
    if (!Notify) return;

    // Validate owner
    const AActor* Owner = GetOwner();
    if (!Owner) return;

    // 🛡️ Guard: only run if this character is actively playing this montage
    if (const ACharacter* C = Cast<ACharacter>(Owner);
        !(C && C->GetMesh() && C->GetMesh()->GetAnimInstance() &&
            C->GetMesh()->GetAnimInstance()->Montage_IsPlaying(CurrentAttack.AttackMontage)))
        return;

    switch (EventType)
    {
        case EMCS_AnimEventType::HitboxWindow:
            // Stop hit detection for this hitbox
            if (CachedHitboxComp)
                CachedHitboxComp->StopHitDetection();
            break;

        case EMCS_AnimEventType::ComboWindow:
            // Close combo window
            bIsComboWindowOpen = false;

            // Fire the combo end event
            OnComboWindowEnd.Broadcast();

            // If combo was open but no input triggered next attack, reset
            if (!bCanContinueCombo)
                AllowedComboNames.Reset();

            break;

        case EMCS_AnimEventType::ParryWindow:
            OnParryWindowEnd.Broadcast(const_cast<AActor*>(Owner));
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Parry Window End for %s"), *Owner->GetName());
            break;

        case EMCS_AnimEventType::DefenseWindow:
            OnDefenseWindowEnd.Broadcast(const_cast<AActor*>(Owner));
            UE_LOG(LogTemp, Log, TEXT("[CombatCore] Defense Window End for %s"), *Owner->GetName());
            break;
    }
}

/**
 * Sets the active attack DataTable using a gameplay tag.
 */
bool UMCS_CombatCoreComponent::SetActiveAttackSet(const FGameplayTag& NewAttackSetTag)
{
    const FMCS_AttackSetData* FoundSet = AttackSets.Find(NewAttackSetTag);
    if (!FoundSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] No AttackSet found for tag: %s"), *NewAttackSetTag.ToString());
        return false;
    }

    if (!FoundSet->AttackDataTable || !FoundSet->AttackChooser)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] AttackSet '%s' missing DataTable or Chooser Class."), *NewAttackSetTag.ToString());
        return false;
    }

    ActiveAttackSetTag = NewAttackSetTag;
    AttackDataTable = FoundSet->AttackDataTable;

    //----------------------------------------
    // 🧩 Create a runtime instance from the class
    //----------------------------------------
    ActiveAttackChooser = NewObject<UMCS_AttackChooser>(this, FoundSet->AttackChooser);
    if (!IsValid(ActiveAttackChooser))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatCore] Failed to instantiate AttackChooser for set: %s"), *NewAttackSetTag.ToString());
        return false;
    }

    //----------------------------------------
    // Load DataTable rows into the chooser
    //----------------------------------------
    ActiveAttackChooser->AttackEntries.Reset();

    TArray<FMCS_AttackEntry*> Rows;
    AttackDataTable->GetAllRows(TEXT("LoadFromSet"), Rows);

    for (FMCS_AttackEntry* Row : Rows)
    {
        if (Row)
        {
            ActiveAttackChooser->AttackEntries.Add(*Row);
        }
    }

    return true;
}

/**
 * Gets the currently active attack DataTable.
 */
UDataTable* UMCS_CombatCoreComponent::GetActiveAttackTable() const
{
    if (const FMCS_AttackSetData* Found = AttackSets.Find(ActiveAttackSetTag))
    {
        return Found->AttackDataTable;
    }
    return nullptr;
}

/**
 * Update Player Situation
 * @param DeltaTime - time since last update
 */
void UMCS_CombatCoreComponent::UpdatePlayerSituation(float DeltaTime)
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    ACharacter* Character = Cast<ACharacter>(OwnerPawn);
    UCharacterMovementComponent* MoveComp = Character ? Character->GetCharacterMovement() : nullptr;

    // --- Basic Movement Flags ---
    PlayerSituation.bIsGrounded = MoveComp ? MoveComp->IsMovingOnGround() : true;
    PlayerSituation.bIsInAir = MoveComp ? MoveComp->IsFalling() : false;
    PlayerSituation.bIsRunning = MoveComp ? MoveComp->Velocity.Size() > 300.f : false;
    PlayerSituation.bIsCrouching = Character ? Character->bIsCrouched : false;

    // --- Advanced Combat States ---
    PlayerSituation.bIsCountering = false; // These can later be set by animation notifies or gameplay events
    PlayerSituation.bIsParrying = false;
    PlayerSituation.bIsRiposting = false;
    PlayerSituation.bIsFinishing = false;

    // --- Quantitative Stats ---
    PlayerSituation.Speed = MoveComp ? MoveComp->Velocity.Size() : 0.f;

    // Altitude = actor's height above the nearest ground trace
    FHitResult Hit;
    const FVector Start = OwnerPawn->GetActorLocation();
    const FVector End = Start - FVector(0.f, 0.f, 10000.f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        PlayerSituation.Altitude = (Start - Hit.Location).Size();
    else
        PlayerSituation.Altitude = 0.f;

    // Check for parry/blocking state from defense component
    if (UMCS_CombatDefenseComponent* Defense = OwnerPawn->FindComponentByClass<UMCS_CombatDefenseComponent>())
    {
        PlayerSituation.bIsParrying = Defense->bIsInParryWindow; // Is parrying state.
        PlayerSituation.bIsBlocking = Defense->bIsInDefenseWindow; // Is blocking state.
    }

    // Optional: get stamina/health percent from owner’s interface or attributes (placeholder)
    PlayerSituation.Stamina = 100.f;
    PlayerSituation.HealthPercent = 100.f;
}

UMCS_AttackChooser* UMCS_CombatCoreComponent::GetPooledChooser(TSubclassOf<UMCS_AttackChooser> ChooserClass)
{
    // Try to find an existing valid chooser of the same class
    for (UMCS_AttackChooser* Existing : ChooserPool)
    {
        if (IsValid(Existing) && Existing->GetClass() == ChooserClass)
        {
            // Reset entries to ensure a clean state before reuse
            Existing->AttackEntries.Reset();
            return Existing;
        }
    }

    // None found — create a new one and add to pool
    UMCS_AttackChooser* NewChooser = NewObject<UMCS_AttackChooser>(this, ChooserClass);
    ChooserPool.Add(NewChooser);

    UE_LOG(LogTemp, Verbose, TEXT("[CombatCore] Added new chooser instance to pool: %s"), *ChooserClass->GetName());
    return NewChooser;
}


#if WITH_EDITORONLY_DATA
/**
 * Draws the Motion Combat System debug overlay.
 * Call from your PlayerController::DrawHUD() or custom AHUD::DrawHUD().
 */
void UMCS_CombatCoreComponent::DrawDebugOverlay(FCanvas* Canvas, float& Y) const
{
    if (!Canvas) return;

    const FMCS_AttackSetData* ActiveSet = AttackSets.Find(ActiveAttackSetTag);
    if (!ActiveSet || !ActiveSet->AttackChooser) return;

    const TArray<FMCS_DebugAttackScore>& Scores = ActiveAttackChooser->DebugScores;
    if (Scores.IsEmpty()) return;

    const float X = 50.f;
    const float LineHeight = 14.f;

    //----------------------------------------
    // Header
    //----------------------------------------
    {
        const FString Header = FString::Printf(
            TEXT("Motion Combat Debug - Active Set: %s"),
            *ActiveAttackSetTag.ToString());

        FCanvasTextItem HeaderItem(FVector2D(X, Y),
            FText::FromString(Header),
            GEngine->GetMediumFont(),
            FLinearColor(FColor::Cyan));

        HeaderItem.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(HeaderItem);
        Y += 22.f;
    }

    //----------------------------------------
    // Attack Entries
    //----------------------------------------
    for (const FMCS_DebugAttackScore& Info : Scores)
    {
        const FLinearColor Color =
            Info.bWasChosen ? FLinearColor::Yellow : FLinearColor::White;

        const FString Line = FString::Printf(
            TEXT("%s | Total: %.1f [B%.1f T%.1f D%.1f Dir%.1f Sit%.1f]"),
            *Info.AttackName.ToString(),
            Info.TotalScore,
            Info.BaseScore,
            Info.TagScore,
            Info.DistanceScore,
            Info.DirectionScore,
            Info.SituationScore);

        FCanvasTextItem LineItem(FVector2D(X, Y),
            FText::FromString(Line),
            GEngine->GetTinyFont(),
            Color);

        LineItem.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(LineItem);

        Y += LineHeight;
    }

    //----------------------------------------
    // Player Situation Summary
    //----------------------------------------
    Y += 20.f;
    {
        const FString SituationHeader = TEXT("Current Player Situation:");
        FCanvasTextItem SitHeader(FVector2D(X, Y),
            FText::FromString(SituationHeader),
            GEngine->GetMediumFont(),
            FLinearColor::Green);
        SitHeader.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(SitHeader);
        Y += 18.f;

        auto DrawBool = [ & ] (const FString& Label, bool bValue)
            {
                const FLinearColor Color = bValue ? FLinearColor::Green : FLinearColor::Red;
                const FString Line = FString::Printf(TEXT("%-12s : %s"), *Label, bValue ? TEXT("True") : TEXT("False"));
                FCanvasTextItem Item(FVector2D(X, Y),
                    FText::FromString(Line),
                    GEngine->GetTinyFont(),
                    Color);
                Canvas->DrawItem(Item);
                Y += LineHeight;
            };

        auto DrawFloat = [ & ] (const FString& Label, float Value)
            {
                const FString Line = FString::Printf(TEXT("%-12s : %.1f"), *Label, Value);
                FCanvasTextItem Item(FVector2D(X, Y),
                    FText::FromString(Line),
                    GEngine->GetTinyFont(),
                    FLinearColor::Yellow);
                Canvas->DrawItem(Item);
                Y += LineHeight;
            };

        // Qualitative flags
        DrawBool(TEXT("Is Grounded"), PlayerSituation.bIsGrounded);
        DrawBool(TEXT("Is In Air"), PlayerSituation.bIsInAir);
        DrawBool(TEXT("Is Running"), PlayerSituation.bIsRunning);
        DrawBool(TEXT("Is Crouching"), PlayerSituation.bIsCrouching);
        DrawBool(TEXT("Is Countering"), PlayerSituation.bIsCountering);
        DrawBool(TEXT("Is Parrying"), PlayerSituation.bIsParrying);
        DrawBool(TEXT("Is Riposting"), PlayerSituation.bIsRiposting);
        DrawBool(TEXT("Is Finishing"), PlayerSituation.bIsFinishing);

        Y += 5.f;

        // Quantitative values
        DrawFloat(TEXT("Speed"), PlayerSituation.Speed);
        DrawFloat(TEXT("Altitude"), PlayerSituation.Altitude);
        DrawFloat(TEXT("Stamina"), PlayerSituation.Stamina);
        DrawFloat(TEXT("Health %"), PlayerSituation.HealthPercent);
    }
}
#endif
