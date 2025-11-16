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
 * MC_SetFocusTask.h
 * A StateTree task that sets the focus on a actor.
 */

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreePropertyBindings.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_SetFocusTask.generated.h"


/**
   * StateTree instance data for the Set Focus task.
  * This struct holds the context data for the task, such as the actor to focus on.
 */
USTRUCT(BlueprintType, Category = "Motion Combat|State Tree|Tasks",
    meta = (DisplayName = "Set Focus Task Instance Data",
        Description = "Instance data for the set focus StateTree task.",
        ToolTip = "Instance data for the Set Focus StateTree task.")
)
struct FInstanceData
{
    GENERATED_BODY()

    /** The enemy character this task is associated with. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to Actor.
    TObjectPtr<AMC_CharacterBase> Actor;

    /** The AI controller managing the enemy character. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to AIController.
    TObjectPtr<AMC_EnemyAIController> AIController;

    /** The actor to focus on */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Focus Actor", ToolTip = "The actor that the AI should focus on."))
    TObjectPtr<AActor> FocusActor = nullptr;

    /** Whether to set focus on the actor */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Set Focus", ToolTip = "If true, the AI will set focus on the specified actor. If false, it will clear focus."))
    bool bSetFocusOnActor = true;
};

/**
  * A StateTree task that sets the focus on a actor.
*/
USTRUCT(category = "Motion Combat|State Tree|Tasks",
    meta = (
        DisplayName = "Set Focus Task",
        ToolTip = "A task that sets the focus on a specified actor.",
        Keywords = "Focus, AI, Task")
)
struct MOTIONCOMBAT_API FMC_SetFocusTask : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    // Tell the runtime what instance‐data type we’re using
    using FInstanceDataType = FInstanceData;

    /**
     * Returns the instance data type for this task.
     */
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceData::StaticStruct(); }

    /**
     * Called when entering the state
     * @param Context The execution context for the state tree
     * @param Transition The transition that triggered this state
     */
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
    {
        const FInstanceData& Data = Context.GetInstanceData(*this);

        if (!IsValid(Data.AIController))
        {
            return EStateTreeRunStatus::Failed;
        }

        if (!IsValid(Data.FocusActor))
        {
            Data.AIController->ClearFocus(EAIFocusPriority::Gameplay);
            return EStateTreeRunStatus::Failed;
        }

        Data.AIController->SetFocus(Data.FocusActor, Data.bSetFocusOnActor);

        if (Data.bSetFocusOnActor)
        {
            Data.AIController->SetFocus(Data.FocusActor, Data.bSetFocusOnActor);
        }
        else
        {
            Data.AIController->ClearFocus(EAIFocusPriority::Gameplay);
        }

        return EStateTreeRunStatus::Succeeded;
    }

#if WITH_EDITOR

    virtual FName GetIconName() const override { return FName("GenericPlay"); }
    virtual FColor GetIconColor() const override { return FColor::Silver; }
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
    {
        const FInstanceData* Data = InstanceDataView.GetPtr<FInstanceData>();
        if (!Data)
        {
            return NSLOCTEXT("MotionCombat", "SetFocusTask_NoData", "Set Focus Task");
        }

        // Build the binding path for FocusActor
        const FPropertyBindingPath InstancePath(ID, GET_MEMBER_NAME_CHECKED(FInstanceData, FocusActor));
        const FPropertyBindingPath* SourcePath = BindingLookup.GetPropertyBindingSource(InstancePath);

        FString BoundName;
        if (SourcePath && SourcePath->NumSegments() > 0)
        {
            for (int32 SegmentIndex = 0; SegmentIndex < SourcePath->NumSegments(); ++SegmentIndex)
            {
                if (SegmentIndex > 0)
                {
                    BoundName.AppendChar(TEXT('.'));
                }
                const FPropertyBindingPathSegment& Segment = SourcePath->GetSegment(SegmentIndex);
                BoundName += Segment.GetName().ToString();
            }
        }
        else
        {
            BoundName = TEXT("[Bound Parameter]");
        }

        return FText::Format(
            NSLOCTEXT("MotionCombat", "SetFocusTaskDesc", "Set Focus to Actor {0}"),
            FText::FromString(BoundName)
        );
    }
    
#endif
};
