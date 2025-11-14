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
 * Date: 11-11-2025
 * =============================================================================
 * MC_GetRandomLocation.cpp
 * Implements a StateTree Task that returns a random location within a radius.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_GetRandomLocation.generated.h"


/**
 * Instance data for the Get Random Location task.
 * Contains the context objects and inputs/outputs required at runtime.
 */
USTRUCT(BlueprintType, Category = "Motion Combat|State Tree|Tasks",
    meta = (DisplayName = "Get Random Location Task Instance Data",
        Description = "Instance data for the Get Random Location StateTree task.",
        ToolTip = "Instance data for the Get Random Location StateTree task."))
    struct FGetRandomLocationTaskInstanceData
{
    GENERATED_BODY()

    /** The enemy character this task is associated with. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to Actor.
    TObjectPtr<AMC_CharacterBase> Actor;

    /** The AI controller managing the enemy character. */
    UPROPERTY(BlueprintReadOnly, Category = "Context", meta = (Input)) // Context category and meta Input allows for binding, but auto binds to AIController.
    TObjectPtr<AMC_EnemyAIController> AIController;

    /** Search radius in world units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SearchRadius = 1000.0f;

    /** Output: a random reachable location. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Output)) // Marked as Output for StateTree allows for binding to parameters.
    FVector RandomLocation = FVector::ZeroVector;
};

/**
 * StateTree Task:
 * Attempts to locate a random reachable position within SearchRadius units
 * around the Actor and outputs the result.
 */
USTRUCT(Category = "Motion Combat|State Tree|Tasks",
    meta = (DisplayName = "Get Random Location",
        Description = "StateTree task to get a random reachable location within a radius around an actor.",
        ToolTip = "StateTree task to get a random reachable location within a radius around an actor.",
        Keywords = "Get Random Location, AI, Navigation, StateTree")
)
struct MOTIONCOMBAT_API FMC_GetRandomLocation : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    // Tell the runtime what instance‐data type we’re using
    using FInstanceDataType = FGetRandomLocationTaskInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FGetRandomLocationTaskInstanceData::StaticStruct(); }

    /**
     * Called when the task is entered.
     * @param Context The execution context.
     * @param Transition The transition result.
     */
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
    {
        // Get the instance data for this task
        FGetRandomLocationTaskInstanceData& Data = Context.GetInstanceData(*this);
        
        // Safety checks
        if (!Data.Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: Actor is null."));
            return EStateTreeRunStatus::Failed;
        }

        UWorld* World = Data.Actor->GetWorld();
        if (!World)
        {
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: World is null."));
            return EStateTreeRunStatus::Failed;
        }

        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!NavSys)
        {
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: NavSys is null."));
            return EStateTreeRunStatus::Failed;
        }

        FVector Origin = Data.Actor->GetActorLocation();
        FNavLocation NavLocation;

        const bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Data.SearchRadius, NavLocation);
        if (bFound)
        {
            Data.RandomLocation = NavLocation.Location;
            return EStateTreeRunStatus::Succeeded;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: Could not find random location."));
            return EStateTreeRunStatus::Failed;
        }
    }

#if WITH_EDITOR
    // Optional editor polish.
    virtual FName GetIconName() const override { return FName("GenericPlay"); }
    virtual FColor GetIconColor() const override { return FColor(128, 200, 255); }

    virtual FText GetDescription(const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting) const override
    {
        const FGetRandomLocationTaskInstanceData* Data = InstanceDataView.GetPtr<FGetRandomLocationTaskInstanceData>();
        if (Data)
        {
            return FText::Format(
                NSLOCTEXT("MotionCombat", "GetRandomLocationDescriptionDetailed", "Get random location around {0}, radius: {1} units"),
                FText::FromString("Actor"), // Placeholder for Actor name, if needed
                FText::AsNumber(Data->SearchRadius)
            );
        }

        return NSLOCTEXT("MotionCombat", "DescRandomLocation_NoData", "Find random reachable location");
    }
#endif // WITH_EDITOR
};
