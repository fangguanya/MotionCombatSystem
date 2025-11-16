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
 * Date: 11-15-2025
 * =============================================================================
 * MC_HealthComponent.h
 * Extended health component with automatic world-space healthbar widget
 * attached to a skeletal socket. Provides billboard behavior, hide/show
 * rules, and Blueprint events for UI customization.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "MC_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMCSHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMCSDamageTaken, float, Damage, float, NewHealth, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMCSDeath, AActor*, DeadActor);

/**
 * Health component for all MC characters / enemies.
 * - Holds CurrentHealth & MaxHealth
 * - Handles ApplyDamage, Heal, Kill functions
 * - Broadcasts events for HealthChanged, DamageTaken, and Death
 */
UCLASS(Blueprintable, ClassGroup=(MotionCombat), meta=(BlueprintSpawnableComponent, DisplayName="Motion Combat Health Component"))
class MOTIONCOMBAT_API UMC_HealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMC_HealthComponent();

    /*
     *  Properties
     */

    /** Maximum health value */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Combat|Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.f;

    /** Current health value */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Combat|Health")
    float CurrentHealth = 100.f;

    /** True when actor has hit zero health */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Combat|Health")
    bool bIsDead = false;

    /** Broadcast whenever health changes (damage or healing) */
    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|Health")
    FOnMCSHealthChanged OnHealthChanged;

    /** Broadcast whenever damage is taken */
    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|Health")
    FOnMCSDamageTaken OnDamageTaken;

    /** Broadcast once when the actor dies */
    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|Health")
    FOnMCSDeath OnDeath;

    /*
     *  Functions
     */

    /** Apply damage to this actor */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Health")
    virtual void ApplyDamage(float DamageAmount, AActor* InstigatorActor);

    /** Heal the actor */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Health")
    virtual void Heal(float HealAmount);

    /** Set health to full */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Health")
    virtual void RestoreToFull();

    /** Instantly kill the actor */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Health")
    virtual void Kill(AActor* InstigatorActor);

    /** Returns true if health <= 0 */
    UFUNCTION(BlueprintCallable, Category="Motion Combat|Health")
    bool IsDead() const { return bIsDead; }

protected:

    /*
     *  Functions
     */

    // Called when the game starts
    virtual void BeginPlay() override;

    // Called when the game ends
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

    /*
     *  Functions
     */
    
    /** Internal helper to handle death logic once */
    void HandleDeath(AActor* InstigatorActor);

    // Clamps CurrentHealth between 0 and MaxHealth
    void ClampHealth();
};
