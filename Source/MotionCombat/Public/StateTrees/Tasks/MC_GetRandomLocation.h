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
<<<<<<< HEAD
 * Date: 11-13-2025
 * =============================================================================
 * MC_GetRandomLocation.h
 * A StateTree task that searches for a reachable random location within a radius
 * around the specified actor and outputs the result.
=======
 * Date: 11-11-2025
 * =============================================================================
 * MC_GetRandomLocation.cpp
 * Implements a StateTree Task that returns a random location within a radius.
>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "NavigationSystem.h"
<<<<<<< HEAD
=======
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
#include "GameFramework/Actor.h"
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "MC_GetRandomLocation.generated.h"


/**
<<<<<<< HEAD
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

    /*
     * Called when the task enters its state.
     * Generates a random reachable location and stores it in RandomLocation output.
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
=======
 * UMC_GetRandomLocation
 *
 * A lightweight StateTree Task that returns a random world-space point
 * within a specified radius around a provided actor (or world origin if none).
 *
 * - Does NOT require navigation (UseNavMesh=false equivalent).
 * - Ideal for simple wander or spacing logic.
 */
UCLASS(Blueprintable, category = "Motion Combat|State Tree|Tasks", meta = (DisplayName = "Get Random Location"))
class MOTIONCOMBAT_API UMC_GetRandomLocation : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:
    /** Default constructor (must use FObjectInitializer for StateTree tasks). */
    UMC_GetRandomLocation(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
        : Super(ObjectInitializer)
        , Actor(nullptr)
        , Radius(500.f)
        , RandomLocation(FVector::ZeroVector)
    {
    }

protected:

    /** The enemy character this task is associated with. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Context, meta = (Context))
    TObjectPtr<AMC_CharacterBase> Actor;

    /** The AI controller managing the enemy character. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Context, meta = (Context))
    TObjectPtr<AMC_EnemyAIController> AIController;

    /** Radius (in world units) to generate the random point within. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Location", meta = (Input, ClampMin = "0.0", Tooltip = "Radius to generate the random point within."))
    float Radius;

    /** The generated random location. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Location", meta = (Output, ExposeOnSpawn = true, Tooltip = "The generated random world location."))
    FVector RandomLocation;

public:

    /**
     * Called when the task is entered.
     * @param Context The execution context.
     * @param Transition The transition result.
     */
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override
    {
        // Safety checks
        if (!Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: Actor is null."));
            return EStateTreeRunStatus::Failed;
        }

        UWorld* World = Actor->GetWorld();
        if (!World)
        {
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: World is null."));
>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
            return EStateTreeRunStatus::Failed;
        }

        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!NavSys)
        {
<<<<<<< HEAD
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: NavSys is null."));
            return EStateTreeRunStatus::Failed;
        }

        FVector Origin = Data.Actor->GetActorLocation();
        FNavLocation NavLocation;

        const bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Data.SearchRadius, NavLocation);
        if (bFound)
        {
            Data.RandomLocation = NavLocation.Location;
=======
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: NavSys is null."));
            return EStateTreeRunStatus::Failed;
        }

        FVector Origin = Actor->GetActorLocation();
        FNavLocation NavLocation;

        const bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLocation);
        if (bFound)
        {
            RandomLocation = NavLocation.Location;
>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
            return EStateTreeRunStatus::Succeeded;
        }
        else
        {
<<<<<<< HEAD
            UE_LOG(LogTemp, Warning, TEXT("PGAS_GetRandomLocation: Could not find random location."));
=======
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: Could not find random location."));
>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
            return EStateTreeRunStatus::Failed;
        }
    }

#if WITH_EDITOR
<<<<<<< HEAD
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
=======

    /** Provides a description for the task in the editor. */
    virtual FText GetDescription(const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override
    {
        return FText::Format(
            NSLOCTEXT("PGAS", "GetRandomLocationDescriptionDetailed", "Get random location around {0}, radius: {1} units"),
            FText::FromString("Actor"), // Placeholder for Actor name, if needed
            FText::AsNumber(Radius)
        );
    }

>>>>>>> 902d32c0cd9fd8fe7853cd9d04ad2d494ac8afd8
#endif
};
