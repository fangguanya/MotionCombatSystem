/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat System
 * Author: Christopher D. Parker
 * Date: 11-11-2025
 * =============================================================================
 * MC_AINodeLibrary.h
 * Blueprint function library exposing helper nodes for AI logic flow.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include <Enums/MC_AIStateEnums.h>
#include <AI/Nodes/MC_AINodeBase.h>
#include "MC_AINodeLibrary.generated.h"

 /**
  * Provides high-level Blueprint nodes that can be used
  * to implement enemy AI logic flow directly inside the AIC_Enemy Blueprint.
  */
UCLASS()
class MOTIONCOMBAT_API UMC_AINodeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** Starts an AI node (calls OnEnterNode on the provided node). */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes", meta = (WorldContext = "WorldContextObject"))
    static void StartAINode(UObject* WorldContextObject, UMC_AINodeBase* Node, AAIController* Controller, APawn* Pawn);

    /** Waits for a number of seconds before triggering a delegate. (Non-blocking latent Blueprint node) */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", DisplayName = "Wait (AI Node)"))
    static void WaitForSeconds(UObject* WorldContextObject, FLatentActionInfo LatentInfo, float Duration);

    /** Moves the AI pawn to a target actor. Returns success if Move reached goal. */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes", meta = (WorldContext = "WorldContextObject"))
    static bool MoveToTarget(UObject* WorldContextObject, AAIController* Controller, AActor* TargetActor, float AcceptanceRadius = 50.f);

    /** Executes a simple “attack target” using the combat core component (if present). */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes", meta = (WorldContext = "WorldContextObject"))
    static bool AttackTarget(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor);

    /** Changes the AI's current high-level state (for display/debug). */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes", meta = (WorldContext = "WorldContextObject"))
    static void ChangeAIState(UObject* WorldContextObject, AAIController* Controller, EMC_AIState NewState);

    	/** Finds a random reachable location within radius from the given origin. */
	UFUNCTION(BlueprintCallable, Category="Motion Combat|AI Nodes", meta=(WorldContext="WorldContextObject"))
	static bool FindRandomPatrolLocation(UObject* WorldContextObject, const FVector& Origin, float Radius, FVector& OutLocation);

	/** Moves the controlled pawn to a random reachable location within radius. */
	UFUNCTION(BlueprintCallable, Category="Motion Combat|AI Nodes", meta=(WorldContext="WorldContextObject"))
	static bool MoveToRandomLocation(UObject* WorldContextObject, AAIController* Controller, float Radius, float AcceptanceRadius = 50.f);

	/** Rotates the controlled pawn to face a target actor smoothly. */
	UFUNCTION(BlueprintCallable, Category="Motion Combat|AI Nodes", meta=(WorldContext="WorldContextObject"))
	static void FaceTarget(UObject* WorldContextObject, APawn* Pawn, AActor* TargetActor, float InterpSpeed = 5.f);

	/** Checks if the AI pawn has line of sight to the target actor. */
	UFUNCTION(BlueprintPure, Category="Motion Combat|AI Nodes", meta=(WorldContext="WorldContextObject"))
	static bool HasLineOfSightToTarget(UObject* WorldContextObject, AAIController* Controller, AActor* TargetActor);

	/** Draws a debug sphere at a world location (for visualization). */
	UFUNCTION(BlueprintCallable, Category="Motion Combat|AI Nodes", meta=(WorldContext="WorldContextObject"))
    static void DrawDebugLocation(UObject* WorldContextObject, const FVector& Location, float Radius = 25.f, FColor Color = FColor::Green, float Duration = 2.f);

    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI Nodes")
    static UMC_AINodeBase* CreateAINode(TSubclassOf<UMC_AINodeBase> NodeClass, UObject* Outer)
    {
        return NewObject<UMC_AINodeBase>(Outer, NodeClass);
    }
};
