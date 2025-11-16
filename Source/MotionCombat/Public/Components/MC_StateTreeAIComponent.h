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
 * MC_StateTreeAIComponent.h
 * Custom State Tree AI Component for Motion Combat.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTreeInstanceData.h"
#include "GameplayTagContainer.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "StateTreeEvents.h"
#include "StateTreeExecutionContext.h"
#include "MC_StateTreeAIComponent.generated.h"

UCLASS(ClassGroup = (MotionCombat), meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat State Tree AI Component"))
class MOTIONCOMBAT_API UMC_StateTreeAIComponent : public UStateTreeAIComponent
{
    GENERATED_BODY()

public:
    /*
     * Functions
    */

    // Constructor
    UMC_StateTreeAIComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /**
     * Sets the State Tree reference for this component and starts the logic.
     * @param StateTreeReference The State Tree reference to set.
    */
    UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (AutoCreateRefTerm = "StateTreeReference", DisplayName = "Start State Tree"))
    void StartStateTree(UStateTree* StateTree);

    /**
     * Stops the currently running State Tree logic.
     */
    UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (DisplayName = "Stop State Tree"))
    void StopStateTree();

    /**
     * Sends an event to the State Tree.
     * @param EventTag The gameplay tag representing the event to send.
    */
    UFUNCTION(BlueprintCallable, Category = "StateTree", meta = (AutoCreateRefTerm = "EventTag", DisplayName = "Send Event"))
    void SendEvent(const FGameplayTag& EventTag);

protected:

    /*
     * Functions
    */


    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

    // Called every frame (Uncomment if ticking is needed)
    // virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Called when the component is initialized */
    virtual void InitializeComponent() override;

    /** Validates the State Tree reference */
    virtual void ValidateStateTreeReference() override;

    /** Called when the controller is ending play */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

    /*
     * Properties
    */

    /** The last event tag sent to the State Tree */
    FGameplayTag LastEventTag = FGameplayTag::EmptyTag;
};