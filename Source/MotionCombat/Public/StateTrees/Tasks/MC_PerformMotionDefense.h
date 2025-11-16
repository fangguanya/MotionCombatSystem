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
 * MC_PerformMotionDefense.h
 * State Tree task that performs a motion defense.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_PerformMotionDefense.generated.h"


 /**
  * Performs a motion defense when the task enters the state.
  */
UCLASS(Blueprintable, BlueprintType, Category = "Motion Combat|State Tree|Tasks", meta = (DisplayName = "Perform Motion Defense"))
class MOTIONCOMBAT_API UMC_PerformMotionDefense : public UStateTreeTaskBlueprintBase
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

    /** Defend (Block, dodge, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    bool Defense = true;

    /** Parry */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
    bool Parry = false;

    /*
     * Functions
    */

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
    {
        return NSLOCTEXT(
            "MotionCombat",
            "PerformMotionDefense",
            "Perform Motion Combat System Defense"
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

        // Get the combat defense component
        UMCS_CombatDefenseComponent* DefenseCore = Actor->GetCombatDefenseComponent();
        if (!IsValid(DefenseCore))
            return EStateTreeRunStatus::Failed;

        if(Defense)
        {        
            DefenseCore->TryDefense(); // Perform the defense
        }
        else if(Parry)
        {
            DefenseCore->TryParry(); // Perform the parry
        }

        // Successfully set the speed
        return EStateTreeRunStatus::Succeeded;
    }
};
