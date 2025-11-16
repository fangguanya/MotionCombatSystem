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
 * MC_UpdateMovementSpeed.h
 * State Tree task that changes an MC_CharacterBase’s MaxWalkSpeed.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_UpdateMovementSpeed.generated.h"


 /**
  * Updates a character’s movement speed when the task enters the state.
  */
UCLASS(Blueprintable, BlueprintType, Category = "Motion Combat|State Tree|Tasks", meta = (DisplayName = "Update Movement Speed"))
class MOTIONCOMBAT_API UMC_UpdateMovementSpeed : public UStateTreeTaskBlueprintBase
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

    /** Desired MaxWalkSpeed to apply while this task is active. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
    float TargetMaxWalkSpeed = 600.0f;

    /*
     * Functions
    */

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
    {
        return FText::Format(
            NSLOCTEXT("MotionCombat", "UpdateMovementSpeed", "Set Actor Max Walk Speed to {0}"), FText::AsNumber(TargetMaxWalkSpeed)
        );
    }
#endif

protected:
    /*
     * Functions
    */

    /**
     * This function sets the character's MaxWalkSpeed to the TargetMaxWalkSpeed when entering the state.
     * @param Context The execution context for the state tree.
     * @param Transition The transition result containing information about the state change.
    */
    EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
    {
        Super::EnterState(Context, Transition);

        // Safety checks
        if (!Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("MC_UpdateMovementSpeed: Actor is null."));
            return EStateTreeRunStatus::Failed;
        }

        // Get the character movement component
        UCharacterMovementComponent* MoveComp = Actor->GetCharacterMovement();
        if (!IsValid(MoveComp))
            return EStateTreeRunStatus::Failed;

        // Set the desired MaxWalkSpeed
        MoveComp->MaxWalkSpeed = TargetMaxWalkSpeed;
        
        // Successfully set the speed
        return EStateTreeRunStatus::Succeeded;
    }
};
