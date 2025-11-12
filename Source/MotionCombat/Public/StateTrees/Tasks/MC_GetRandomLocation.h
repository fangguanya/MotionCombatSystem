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
            return EStateTreeRunStatus::Failed;
        }

        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!NavSys)
        {
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: NavSys is null."));
            return EStateTreeRunStatus::Failed;
        }

        FVector Origin = Actor->GetActorLocation();
        FNavLocation NavLocation;

        const bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLocation);
        if (bFound)
        {
            RandomLocation = NavLocation.Location;
            return EStateTreeRunStatus::Succeeded;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MC_GetRandomLocation: Could not find random location."));
            return EStateTreeRunStatus::Failed;
        }
    }

#if WITH_EDITOR

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

#endif
};
