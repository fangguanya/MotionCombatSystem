

#include <Components/MC_StateTreeAIComponent.h>
#include "StateTreeSchema.h"


// Constructor
UMC_StateTreeAIComponent::UMC_StateTreeAIComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    SetStartLogicAutomatically(true);  // Do not start logic automatically on BeginPlay
}

void UMC_StateTreeAIComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UMC_StateTreeAIComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

/**
 * Called when the controller is ending play.
 * This is where we can clean up any AI-specific logic.
 * @param EndPlayReason The reason for ending play.
 */
void UMC_StateTreeAIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopStateTree();       // Ensures Cleanup() + StopLogic + null refs
    LastEventTag = FGameplayTag::EmptyTag;

    Super::EndPlay(EndPlayReason);
}

/** Called when the component is initialized */
void UMC_StateTreeAIComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

/** Validates the State Tree reference */
void UMC_StateTreeAIComponent::ValidateStateTreeReference()
{
    Super::ValidateStateTreeReference();
}

/**
 * Sets the State Tree reference for this component and starts the logic.
 * @param StateTreeReference The State Tree reference to set.
*/
void UMC_StateTreeAIComponent::StartStateTree(UStateTree* StateTree)
{
    if (StateTree)
    {
        if (IsRunning())
        {
            Cleanup();
        }

        StateTreeRef.SetStateTree(StateTree);
        Super::SetStateTree(StateTree);
        StartLogic();
    }
}

/**
 * Stops the currently running State Tree logic.
 */
void UMC_StateTreeAIComponent::StopStateTree()
{
    if (IsRunning())
    {
        Cleanup();
    }

    StopLogic(FString("Stopped by Code"));
    Super::SetStateTree(nullptr);
    StateTreeRef.SetStateTree(nullptr);
}

/**
 * Sends an event to the State Tree.
 * @param EventTag The gameplay tag representing the event to send.
*/
void UMC_StateTreeAIComponent::SendEvent(const FGameplayTag& EventTag)
{
    if (IsRunning() && LastEventTag != EventTag)
    {
        LastEventTag = EventTag; // Update the last event tag to prevent duplicates
        FStateTreeEvent Event = FStateTreeEvent(EventTag);
        SendStateTreeEvent(Event); // Send the event to the StateTree execution context
    }
}
