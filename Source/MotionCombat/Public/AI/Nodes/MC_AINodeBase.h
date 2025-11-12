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
 * MC_AINodeBase.h
 * Base AI Node class that defines an executable unit of logic for AI Blueprints.
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include <Enums/MC_AIStateEnums.h>
#include "MC_AINodeBase.generated.h"

class AAIController;
class APawn;

/*
 * Delegates
*/

/** Event broadcast when the node finishes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAINodeFinishedSignature, bool, bSucceeded);


/**
 * Base class for AI “Nodes” (logic actions or behaviors).
 * Each node can be started, ticked, and stopped either from C++ or Blueprints.
 * Designed for building flow logic directly inside the AI Controller Blueprint.
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Motion Combat AI Node Base"))
class MOTIONCOMBAT_API UMC_AINodeBase : public UObject
{
    GENERATED_BODY()

public:
    UMC_AINodeBase();

    /** Called when the node is started */
    UFUNCTION(BlueprintNativeEvent, Category = "AI Node", meta = (DisplayName = "On Enter Node"))
    void OnEnterNode(AAIController* OwnerController, APawn* ControlledPawn);
    virtual void OnEnterNode_Implementation(AAIController* OwnerController, APawn* ControlledPawn) {}

    /** Called every frame while this node is active */
    UFUNCTION(BlueprintNativeEvent, Category = "AI Node", meta = (DisplayName = "On Tick Node"))
    void OnTickNode(AAIController* OwnerController, APawn* ControlledPawn, float DeltaTime);
    virtual void OnTickNode_Implementation(AAIController* OwnerController, APawn* ControlledPawn, float DeltaTime) {}

    /** Called when the node is exited or interrupted */
    UFUNCTION(BlueprintNativeEvent, Category = "AI Node", meta = (DisplayName = "On Exit Node"))
    void OnExitNode(AAIController* OwnerController, APawn* ControlledPawn);
    virtual void OnExitNode_Implementation(AAIController* OwnerController, APawn* ControlledPawn) {}

    /** Marks this node as finished (can trigger transition in Blueprint) */
    UFUNCTION(BlueprintCallable, Category = "AI Node", meta = (DisplayName = "Finish Node"))
    void FinishNode(bool bSuccess = true);

    /** Whether this node is currently running */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Node", meta = (DisplayName = "Is Running"))
    bool bIsRunning = false;

    /** Whether this node should tick every frame while active. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Node", meta = (DisplayName = "Should Tick"))
    bool bShouldTick = false;

    UPROPERTY(BlueprintAssignable, Category = "AI Node", meta = (DisplayName = "On Node Finished"))
    FAINodeFinishedSignature OnNodeFinished;

    /** Internal helper to set node running state */
    void SetRunning(bool bRunning);

protected:
private:
};
