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
 * MCS_CoreComponent.h
 * Core combat component that coordinates attack selection, target acquisition,
 * and DataTable-driven attack loading.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include <Structs/MCS_AttackEntry.h>
#include <Structs/MCS_AttackSetData.h>
#include <Structs/MCS_AttackHitbox.h>
#include <SubSystems/MCS_TargetingSubsystem.h>
#include <Choosers/MCS_AttackChooser.h>
#include <AnimNotifyStates/AnimNotifyState_MCSWindow.h>
#include <Components/MCS_CombatHitboxComponent.h>
#include "MCS_CombatCoreComponent.generated.h"


/*
 * Delegates
 */

// Delegate broadcast when the target list is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetingUpdatedSignature, const TArray<FMCS_TargetInfo>&, NewTargetList, int32, NumTargets);

// Delegates for combo window begin events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboWindowBeginSignature);

// Delegate for combo window end events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboWindowEndSignature);

// Delegates for parry window start event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryWindowBeginSignature, AActor*, Attacker);

// Delegate for parry window end event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryWindowEndSignature, AActor*, Attacker);

// Delegates for defense window start event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDefenseWindowBeginSignature, AActor*, Defender);

// Delegate for defense window end event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDefenseWindowEndSignature, AActor*, Defender);


/**
 * Core combat component that coordinates attack selection, target acquisition, and DataTable-driven attack loading.
 */
UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Core Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatCoreComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatCoreComponent();

    // Destructor
    virtual ~UMCS_CombatCoreComponent() override = default;

    /*
     * Properties
     */

    /** Holds multiple attack sets, each keyed by a GameplayTag (e.g. "Weapon.Sword", "Stance.Air") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core", meta = (DisplayName = "Attack Sets"))
    TMap<FGameplayTag, FMCS_AttackSetData> AttackSets;

    /** Current character situation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Core", meta = (DisplayName = "Player Situation"))
    FMCS_AttackSituation PlayerSituation;

    /** Blueprint Event triggered whenever the TargetingSubsystem's target list is updated */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Targeting Updated"))
    FOnTargetingUpdatedSignature OnTargetingUpdated;

    /** Blueprint Event triggered when the combo window begins */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Combo Window Begin"))
    FOnComboWindowBeginSignature OnComboWindowBegin;

    /** Blueprint Event triggered when the combo window ends */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Combo Window End"))
    FOnComboWindowEndSignature OnComboWindowEnd;

    /** Blueprint Event triggered when the parry window begins */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Parry Window Begin"))
    FOnParryWindowBeginSignature OnParryWindowBegin;

    /** Blueprint Event triggered when the parry window ends */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Parry Window End"))
    FOnParryWindowEndSignature OnParryWindowEnd;

    /** Blueprint Event triggered when the defense window begins */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Defense Window Begin"))
    FOnDefenseWindowBeginSignature OnDefenseWindowBegin;

    /** Blueprint Event triggered when the defense window ends */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Core|Events", meta = (DisplayName = "On Defense Window End"))
    FOnDefenseWindowEndSignature OnDefenseWindowEnd;


    /*
     * Functions
     */

    /**
     * Chooses an appropriate attack using AttackChooser and available targets, filtered by type
     * @param DesiredType - type of attack to select
     * @param DesiredDirection - direction of the attack in world space
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core",
        meta = (DisplayName = "Select Attack",
            ToolTip = "Selects an attack entry without executing it. Use only if you need to preview or queue attacks manually."))
    bool SelectAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation);

    /**
     * Plays the selected attack's montage if valid
     * @param DesiredType - type of attack to perform
     * @param DesiredDirection - direction of the attack in world space
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core",
        meta = (DisplayName = "Perform Attack", ToolTip = "Selects and executes an attack. You do not need to call SelectAttack first."))
    void PerformAttack(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation);

    /** Gets the closest valid target via TargetingSubsystem */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core", meta = (DisplayName = "Get Closest Target"))
    AActor* GetClosestTarget(float MaxRange = 2500.f) const;

    /**
     * Utility to convert 2D movement input into an EMCS_AttackDirection enum value
     * @param MoveInput - 2D movement input vector (X=Forward/Backward, Y=Left/Right)
     * @return Corresponding EMCS_AttackDirection value
    */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Attack Direction"))
    EMCS_AttackDirection GetAttackDirection(const FVector2D& MoveInput) const;

    /**
     * Sets the active attack DataTable using a gameplay tag.
     * If successful, returns true and updates the AttackChooser.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Core", meta = (DisplayName = "Set Active Attack Set"))
    bool SetActiveAttackSet(const FGameplayTag& NewAttackSetTag);

    /**
     * Gets the currently active attack DataTable (if any).
     */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Active Attack Table"))
    UDataTable* GetActiveAttackTable() const;

    /**
     * Gets the currently selected attack (if any).
     */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Current Attack"))
    FMCS_AttackEntry GetCurrentAttack() const { return CurrentAttack; }

    UFUNCTION(BlueprintCallable, Category = "MCS|Core", meta = (DisplayName = "Update Player Situation"))
    void UpdatePlayerSituation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "MCS|Core|Combo")
    bool TryContinueCombo(EMCS_AttackType DesiredType, EMCS_AttackDirection DesiredDirection, const FMCS_AttackSituation& CurrentSituation);

#if WITH_EDITORONLY_DATA
    /**
     * Draws the Motion Combat System debug overlay.
     * Call from your PlayerController::DrawHUD() or custom AHUD::DrawHUD().
     */
    void DrawDebugOverlay(FCanvas* Canvas, float& Y) const;
#endif

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Update PlayerSituation each frame */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    
    /*
     * Properties
     */
    
    /** Utility to get the owning actor safely */
    AActor* GetOwnerActor() const;

    /** DataTable containing FMCS_AttackEntry definitions */
    UPROPERTY()
    TObjectPtr<UDataTable> AttackDataTable;

    /** Cached reference to the world’s targeting subsystem */
    UPROPERTY()
    TObjectPtr<UMCS_TargetingSubsystem> TargetingSubsystem;

    /** Currently selected attack (if any) */
    UPROPERTY()
    FMCS_AttackEntry CurrentAttack;

    /** The currently active attack set tag */
    UPROPERTY()
    FGameplayTag ActiveAttackSetTag;

    /** Cached list of hitbox window data parsed from the current montage */
    TArray<FMCS_AttackHitbox> CachedHitboxWindows;

    /** Cached pointer to owner’s hitbox component */
    TObjectPtr<UMCS_CombatHitboxComponent> CachedHitboxComp;

    // Keep track of which notify CDOs we’ve bound so we can unbind safely
    UPROPERTY()
    TArray<TObjectPtr<UAnimNotifyState_MCSWindow>> BoundMCSNotifies;

    /** Whether the player is inside an active combo window (set by AnimNotify) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Core|Combo", meta = (AllowPrivateAccess = "true"))
    bool bIsComboWindowOpen = false;

    /** Tracks whether the player can chain into another attack */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Core|Combo", meta = (AllowPrivateAccess = "true"))
    bool bCanContinueCombo = false;

    /** Names of attacks that can follow the current one (populated from AllowedNextAttacks) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Core|Combo", meta = (AllowPrivateAccess = "true"))
    TArray<FName> AllowedComboNames;

    /** Pool of reusable chooser instances to avoid allocations every selection */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMCS_AttackChooser>> ChooserPool;

    /*
     * Functions
     */

    // Handler for TargetingSubsystem target updates
    UFUNCTION()
    void HandleTargetsUpdated(const TArray<FMCS_TargetInfo>& NewTargets, int32 NewTargetCount);

    // Subscribe/unsubscribe to all hitbox notifies used by a montage
    void BindNotifiesForMontage(UAnimMontage* Montage);
    void UnbindAllNotifies();

    // Callback targets for notify broadcasts
    UFUNCTION()
    void HandleMCSNotifyBegin(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* Notify);
    UFUNCTION()
    void HandleMCSNotifyEnd(EMCS_AnimEventType EventType, UAnimNotifyState_MCSWindow* Notify);

    /** Gets a reusable chooser instance or creates a new one if needed */
    UMCS_AttackChooser* GetPooledChooser(TSubclassOf<UMCS_AttackChooser> ChooserClass);

    /** Cleans up invalid chooser instances from the pool */
    void ClearChooserPool()
    {
        for (int32 i = ChooserPool.Num() - 1; i >= 0; --i)
        {
            UMCS_AttackChooser* Chooser = ChooserPool[i];

            // IsValid() handles nullptr and pending kill checks automatically
            if (!IsValid(Chooser))
            {
                ChooserPool.RemoveAt(i);
            }
        }
    }
};
