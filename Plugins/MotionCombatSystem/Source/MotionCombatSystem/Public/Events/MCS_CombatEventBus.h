/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat System
 * Author: Christopher D. Parker
 * Date: 10-27-2025
 * =============================================================================
 * MCS_CombatEventBus.h
 * Global event bus for Motion Combat System – enables decoupled communication
 * between combat components (Core, Defense, HitReaction, etc.)
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include <Structs/MCS_AttackEntry.h>
#include "MCS_CombatEventBus.generated.h"

/**
 * Global event bus for combat-related communication.
 * Exists once per UWorld and provides delegates for broadcasting
 * attacks, parries, blocks, hits, etc.
 */
UCLASS(Blueprintable)
class MOTIONCOMBATSYSTEM_API UMCS_CombatEventBus : public UObject
{
    GENERATED_BODY()

public:

    /**
     * Returns or creates the global Combat Event Bus for this world
     * @param World The UWorld context
     * @return The Combat Event Bus instance for the given world
     */
    static UMCS_CombatEventBus* Get(UWorld* World);

    /**
     * Blueprint-accessible version to get the Combat Event Bus for the given world context
     * @param WorldContextObject The context object to derive the UWorld from
     */
    UFUNCTION(BlueprintCallable, Category = "MCS|Events", meta = (WorldContext = "WorldContextObject"))
    static UMCS_CombatEventBus* GetCombatEventBus(const UObject* WorldContextObject);

    // ==========================================================
    //  Delegates
    // ==========================================================

    /** Attack Start */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackStartedSignature, AActor*, Attacker, AActor*, Target);
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events|Attack", meta=(DisplayName="On Attack Started"))
    FOnAttackStartedSignature OnAttackStarted;

    /** Parry Window Opened */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParryWindowOpenedSignature, AActor*, Attacker, float, Duration);
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events|Parry", meta=(DisplayName="On Parry Window Opened"))
    FOnParryWindowOpenedSignature OnParryWindowOpened;

    /** Parry Success */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnParrySuccessSignature, AActor*, Defender, AActor*, Attacker);
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events|Parry", meta=(DisplayName="On Parry Success"))
    FOnParrySuccessSignature OnParrySuccess;

    /** Block Success */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlockSuccessSignature, AActor*, Defender, AActor*, Attacker);
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events|Block", meta=(DisplayName="On Block Success"))
    FOnBlockSuccessSignature OnBlockSuccess;

    /** Hit Landed */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHitLandedSignature, AActor*, Attacker, AActor*, Defender, const FMCS_AttackEntry&, AttackData);
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events|Hit", meta=(DisplayName="On Hit Landed"))
    FOnHitLandedSignature OnHitLanded;

protected:
    /** Reference cache per world */
    static TMap<TWeakObjectPtr<UWorld>, TObjectPtr<UMCS_CombatEventBus>> BusInstances;
};
