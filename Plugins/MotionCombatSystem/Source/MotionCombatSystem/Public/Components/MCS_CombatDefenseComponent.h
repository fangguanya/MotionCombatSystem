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
UCLASS(Blueprintable, ClassGroup = (MotionCombat), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat System Defense Component"))
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
     * Functions
     */

    UFUNCTION() void HandleParryWindowBegin(AActor* Attacker);
    UFUNCTION() void HandleParryWindowEnd(AActor* Attacker);
    UFUNCTION() void HandleDefenseWindowBegin(AActor* Defender);
    UFUNCTION() void HandleDefenseWindowEnd(AActor* Defender);
};
