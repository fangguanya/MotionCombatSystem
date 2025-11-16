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
 * MC_PerformMotionAttack.h
 * State Tree task that performs a motion attack.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_PerformMotionAttack.generated.h"


 /**
  * Performs a motion attack when the task enters the state.
  */
UCLASS(Blueprintable, BlueprintType, Category = "Motion Combat|State Tree|Tasks", meta = (DisplayName = "Perform Motion Attack"))
class MOTIONCOMBAT_API UMC_PerformMotionAttack : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:
    /*
     * Properties
    */

    /** The enemy character this task is associated with. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to Actor.
        TObjectPtr<AMC_CharacterBase> Actor;

    /** The AI controller managing the enemy character. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to AIController.
        TObjectPtr<AMC_EnemyAIController> AIController;

    /** Desired attack type. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EMCS_AttackType DesiredType = EMCS_AttackType::Light;

    /** Desired attack direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EMCS_AttackDirection DesiredDirection = EMCS_AttackDirection::Forward;


#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
    {
        return NSLOCTEXT(
            "MotionCombat",
            "PerformMotionAttack",
            "Perform Motion Combat System Attack"
        );
    }
#endif

protected:
    /*
     * Functions
    */

    /**
     * This function is called when the task enters the state.
     * @param Context The execution context for the state tree.
     * @param Transition The transition result containing information about the state change.
    */
    EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
    {
        Super::EnterState(Context, Transition);

        // Safety checks
        if (!Actor)
        {
            return EStateTreeRunStatus::Failed;
        }

        if (!AIController)
        {
            return EStateTreeRunStatus::Failed;
        }

        // Get the combat core component
        UMCS_CombatCoreComponent* CombatCore = Actor->GetCombatCoreComponent();
        if (!IsValid(CombatCore))
            return EStateTreeRunStatus::Failed;

        // Setup attack parameters here as needed
        FMCS_AttackSituation CurrentSituation;

        // Perform the attack
        CombatCore->PerformAttack(DesiredType, DesiredDirection, CurrentSituation);

        // Successfully set the speed
        return EStateTreeRunStatus::Succeeded;
    }
};
