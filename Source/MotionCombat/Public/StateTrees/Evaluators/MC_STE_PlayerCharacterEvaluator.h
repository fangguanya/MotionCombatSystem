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
 * MC_STE_PlayerCharacterEvaluator.h
 * Evaluator that monitors the Player and exposes it to the State Tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "Components/StateTreeAIComponent.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_STE_PlayerCharacterEvaluator.generated.h"


/*
 * Evaluator that monitors the Player and exposes it to the State Tree.
 */
UCLASS(Blueprintable,
    meta = (
        DisplayName = "Player Character Evaluator",
        category = "Motion Combat|StateTree|Evaluators",
        ShortTooltip = "Evaluates the health and more of the player character.",
        Tooltip = "Monitors the health and more of the player character."
        )
)
class MOTIONCOMBAT_API UMC_STE_PlayerCharacterEvaluator : public UStateTreeEvaluatorBlueprintBase
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Is Deceased"))
    bool IsDeceased = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Is Attacking"))
    bool  IsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (DisplayName = "Is Defending"))
    bool  IsDefending = false;

    /** Out: The player's current world location */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Output, meta = (StateTreeOutput, DisplayName = "World Position"))
    FVector WorldPosition = FVector::ZeroVector;

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

        if (IsValid(Character) && AIController.IsValid())
        {
            // If the controller no longer has a valid target, exit early. We only want to gather this information when engaged.
            if (!AIController->HasAcquiredTarget())
                return;

            WorldPosition = Character->GetActorLocation();
            CurrentHealth = Character->GetHealth(); // Get the current health from the character base class
            IsDeceased = Character->IsDead(); // Check if the character is dead
            IsAttacking = Character->IsAttacking(); // Check if the character is attacking
            IsDefending = Character->IsDefending(); // Check if the character is defending
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
        IsDeceased = false;
        IsAttacking = false;
        IsDefending = false;
        WorldPosition = FVector::ZeroVector;
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
            "MC_STEPlayerCharacterEvaluator",
            "PlayerCharacterEvaluatorDesc",
            "Monitors the (Player Character) Context Actor"
        );
    };

#endif
};
