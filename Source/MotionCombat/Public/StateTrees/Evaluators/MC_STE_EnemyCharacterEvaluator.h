/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Paragon GAS Sample
 * This is a sample project demonstrating the use of Unreal Engine's Gameplay Ability System (GAS) & open world.
 * Author: Christopher D. Parker
 * Date: 7-31-2025
 * =============================================================================
 * MC_STE_EnemyCharacterEvaluator.h
 * Evaluator that monitors the Enemy and exposes it to the State Tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "Components/StateTreeAIComponent.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_STE_EnemyCharacterEvaluator.generated.h"


/*
 * Evaluator that monitors the Context Actor's health attribute and exposes it to the State Tree as CurrentHealth.
 */
UCLASS(Blueprintable,
    meta = (
        DisplayName = "Enemy Character Evaluator",
        category = "Motion Combat|StateTree|Evaluators",
        ShortTooltip = "Evaluates the health and more of the context actor.",
        Tooltip = "Monitors the health and more of the context actor."
        )
)
class MOTIONCOMBAT_API UMC_STE_EnemyCharacterEvaluator : public UStateTreeEvaluatorBlueprintBase
{
    GENERATED_BODY()

public:

    /*
     * Properties
    */

    /** The owning Enemy Character (set at TreeStart) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context, meta = (Input))
    TObjectPtr<AMC_CharacterBase> Character;

    /** Pointer to the bound controller */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Context, meta = (Input))
    TWeakObjectPtr<AMC_EnemyAIController> AIController;

    /** Out: the actor’s current health. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Current Health"))
    float CurrentHealth = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Am I Deceased"))
    bool AmIDeceased = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Am I Attacking"))
    bool  AmIAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Am I Defending"))
    bool  AmIDefending = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Still Have Target"))
    bool  StillHaveTarget = false;

    /** Out: The enemy's current world location */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (StateTreeOutput, DisplayName = "My Current World Location"))
    FVector MyCurrentWorldLocation = FVector::ZeroVector;

    /*
     * Functions
     */

     /** Cache the context actor and reset health. */
    virtual void TreeStart(FStateTreeExecutionContext& Context) override
    {
        Super::TreeStart(Context);

        ResetEvaluation();
    }

    /** Sample health each tick and write to CurrentHealth. */
    virtual void Tick(FStateTreeExecutionContext& Context, float DeltaTime) override
    {
        Super::Tick(Context, DeltaTime);

        if (IsValid(Character))
        {
            MyCurrentWorldLocation = Character->GetActorLocation();
            CurrentHealth = Character->GetHealth(); // Get the current health from the character base class
            AmIDeceased = Character->IsDead(); // Check if the character is dead
            AmIAttacking = Character->IsAttacking(); // Check if the character is attacking
            AmIDefending = Character->IsDefending(); // Check if the character is defending

            // Check if we have a valid AI Controller
            if (AIController.IsValid())
            {
                StillHaveTarget = AIController->HasAcquiredTarget(); // Check if there is an acquired target
            }
        }
        else
        {
            // If no character is set, reset properties
            ResetEvaluation();
        }
    }

    /** Clear cached pointers. */
    virtual void TreeStop(FStateTreeExecutionContext& Context) override
    {
        Super::TreeStop(Context);
        
        Character = nullptr;
        AIController = nullptr;
    }

    void ResetEvaluation()
    {
        CurrentHealth = 0.f;
        AmIDeceased = false;
        AmIAttacking = false;
        AmIDefending = false;
        StillHaveTarget = false;
        MyCurrentWorldLocation = FVector::ZeroVector;
    }

#if WITH_EDITOR

    /**
     * Gets the description of the condition for editor display.
     * @param ID The unique identifier for the node.
     * @param InstanceDataView The data view for the instance.
     * @param BindingLookup The binding lookup for the state tree.
     * @param Formatting The formatting options for the description.
     * @return The formatted description text.
    */
    virtual FText GetDescription(
        const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const {
        return NSLOCTEXT(
            "MC_STEEnemyCharacterEvaluator",
            "EnemyCharacterEvaluatorDesc",
            "Monitors the (Enemy) Context Actor"
        );
    };

#endif
};
