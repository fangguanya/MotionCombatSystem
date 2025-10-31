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
 * Date: 10-26-2025
 * =============================================================================
 * MCS_CombatDefenseComponent.h
 * Skeleton component responsible for handling defensive actions such as
 * blocking, dodging, ducking, rolling, and defensive reaction montages.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Components/MCS_CombatCoreComponent.h>
#include <Events/MCS_CombatEventBus.h>
#include <Structs/MCS_DefenseEntry.h>
#include <Structs/MCS_DefenseSetData.h>
#include <Engine/DataTable.h>
#include "MCS_CombatDefenseComponent.generated.h"

 /*
  * Delegates
  */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParryOpportunitySignature, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDefenseWindowActiveSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccessSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParryFailSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockSuccessSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockFailSignature);



/**
 * UMCS_CombatDefenseComponent
 *
 * Component that will handle all player and AI defensive actions.
 * For now, it contains only basic setup and lifecycle hooks.
 */
UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Defense Component"))
class MOTIONCOMBATSYSTEM_API UMCS_CombatDefenseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UMCS_CombatDefenseComponent();
    
    // Destructor
    virtual ~UMCS_CombatDefenseComponent() override = default;

    /*
     * Properties
     */

    /**
     * @brief A list of data tables containing defense entries that define all available
     * blocking, dodging, rolling, ducking, and parrying actions for this actor.
     *
     * Each table should use the FMCS_DefenseEntry row structure.
     * Designers can add or remove tables in the editor to modularly expand
     * the available defensive actions per character, weapon, or stance.
     *
     * Example:
     *  - PlayerCharacter_DefenseTable
     *  - Sword_ParryTable
     *  - HeavyArmor_BlockTable
     *
     * @details This array functions similarly to AttackDataTables in the core component,
     * allowing defense logic and chooser systems to aggregate all defensive options
     * at runtime from multiple sources (global, weapon, stance, etc.).
     *
     * @see FMCS_DefenseEntry
     * @see UMCS_DefenseChooser
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Defense", meta = (DisplayName = "Defense Sets"))
    TMap<FGameplayTag, FMCS_DefenseSetData> DefenseSets;

     // ------------------------------
     // State flags
     // ------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Defense")
    bool bIsInParryWindow = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MCS|Defense")
    bool bIsInDefenseWindow = false;

    UPROPERTY(BlueprintReadOnly, Category = "MCS|Defense")
    TObjectPtr<AActor> LastParrySource = nullptr;

    // ------------------------------
    // Blueprint Events
    // ------------------------------

    /** Delegate broadcast when a parry opportunity begins */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Parry Opportunity"))
    FOnParryOpportunitySignature OnParryOpportunity;

    /** Delegate broadcast when a defense window is active */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Defense Window Active"))
    FOnDefenseWindowActiveSignature OnDefenseWindowActive;

    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Parry Success"))
    FOnParrySuccessSignature OnParrySuccess;

    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Parry Fail"))
    FOnParryFailSignature OnParryFail;

    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Block Success"))
    FOnBlockSuccessSignature OnBlockSuccess;

    UPROPERTY(BlueprintAssignable, Category = "MCS|Defense|Events", meta = (DisplayName = "On Block Fail"))
    FOnBlockFailSignature OnBlockFail;

    /*
     * Functions
     */

    UFUNCTION(BlueprintCallable, Category = "MCS|Defense",
        meta = (DisplayName = "Try Parry", ToolTip = "Attempts a parry if timing and facing are valid. Returns true if successful."))
    bool TryParry();

    UFUNCTION(BlueprintCallable, Category = "MCS|Defense",
        meta = (DisplayName = "Try Block", ToolTip = "Attempts to block during a defense window. Returns true if successful."))
    bool TryBlock();

    /**
     * Gets the currently selected attack (if any).
     */
    UFUNCTION(BlueprintPure, Category = "MCS|Core", meta = (DisplayName = "Get Current Defense"))
    FMCS_DefenseEntry GetCurrentDefense() const { return CurrentDefense; }

    /**
     * @brief Sets the active defense set tag and rebuilds the cached defensive pool.
     *
     * Use this when the character’s combat state changes (e.g., weapon swap).
     * The new tag determines which subset of defense data tables are active.
     *
     * @param NewDefenseSetTag  The gameplay tag representing the new active defense set.
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Defense",
        meta = (DisplayName = "Set Active Defense Set",
            ToolTip = "Sets a new active defense set tag and rebuilds available defense options."))
    bool SetActiveDefenseSet(const FGameplayTag& NewDefenseSetTag);

protected:

    /*
     * Functions
     */

     // Called when the game starts
    virtual void BeginPlay() override;

    // Called when the game ends
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

    /*
     * Properties
     */

     /** DataTable containing FMCS_DefenseEntry definitions */
    UPROPERTY()
    TObjectPtr<UDataTable> DefenseDataTable;

    /** Runtime instance of the currently active defense chooser. */
    UPROPERTY(Transient)
    TObjectPtr<UMCS_DefenseChooser> ActiveDefenseChooser = nullptr;

    /** Currently selected defense (if any) */
    UPROPERTY()
    FMCS_DefenseEntry CurrentDefense;

    /** Currently active defense set tag */
    UPROPERTY()
    FGameplayTag ActiveDefenseSetTag;

    /*
     * Functions
     */

    UFUNCTION() void HandleParryWindowBegin(AActor* Attacker);
    UFUNCTION() void HandleParryWindowEnd(AActor* Attacker);
    UFUNCTION() void HandleDefenseWindowBegin(AActor* Defender);
    UFUNCTION() void HandleDefenseWindowEnd(AActor* Defender);

    UFUNCTION() void HandleGlobalAttackStarted(AActor* Attacker, AActor* Target);
    UFUNCTION() void HandleGlobalParryWindowOpened(AActor* Attacker, float Duration);
    UFUNCTION() void HandleGlobalParrySuccess(AActor* Defender, AActor* Attacker);
    UFUNCTION() void HandleGlobalBlockSuccess(AActor* Defender, AActor* Attacker);

};
