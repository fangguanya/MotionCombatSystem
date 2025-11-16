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
 * Date: 7-13-2025
 * =============================================================================
 * MC_EnemyAIController.cpp
 * Custom AI Controller for Enemy Characters (StateTree-ready)
 */

#include <Controllers/MC_EnemyAIController.h>
#include "GameFramework/Pawn.h"
#include "StateTreePropertyRef.h"
#include "StateTreePropertyBindings.h"


 /**
   * Constructor
   * This sets up the AI Controller with perception components and state tree component.
   */
AMC_EnemyAIController::AMC_EnemyAIController()
{
    bAttachToPawn = true; // Attach controller to pawn

    // Set up the State Tree AI Component for managing enemy AI behavior.
    // This component handles the AI behavior for the enemy character.
    StateTreeAIComponent = CreateDefaultSubobject<UMC_StateTreeAIComponent>(TEXT("State Tree AI Component"));

    // Create Perception Component
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));

    // Add sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1000.0f;
    SightConfig->LoseSightRadius = 1500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 35.0f;
    // SightConfig->SetMaxAge(5.f); // How long the perception lasts when player is not seen anymore
    // SightConfig->PointOfViewBackwardOffset = 250.0f; // How far behind the character to check for sight (Peripheral vision)
    // SightConfig->NearClippingRadius = 175.0f; // How close the character can be to still be seen (Peripheral vision)
    SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f; // How far to check for last seen location (Turned off)
    SightConfig->DetectionByAffiliation.bDetectEnemies = true; // GetTeamAttitudeTowards will filter enemies
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false; // GetTeamAttitudeTowards will filter friendlies
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // GetTeamAttitudeTowards will filter neutrals

    PerceptionComponent->ConfigureSense(*SightConfig); // Assign Sight as the sense
    PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass()); // Set Sight as the dominant sense

    // Add hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.f;
    // HearingConfig->SetMaxAge(3.f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    // Add damage perception
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    PerceptionComponent->ConfigureSense(*DamageConfig);

    // Add prediction sense
    PredictionConfig = CreateDefaultSubobject<UAISenseConfig_Prediction>(TEXT("PredictionConfig"));
    PredictionConfig->SetMaxAge(1.0f); // How long the prediction lasts
    PredictionConfig->SetStartsEnabled(true); // Start enabled
    PerceptionComponent->ConfigureSense(*PredictionConfig);

    // Bind perception events
    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMC_EnemyAIController::OnTargetPerceptionUpdated);
    PerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AMC_EnemyAIController::OnTargetPerceptionForgotten);
}

/**
 * Called when the game starts or when spawned.
 * This is where we can initialize any AI-specific logic.
 */
void AMC_EnemyAIController::BeginPlay()
{
    Super::BeginPlay();
}

/*
 * Called every frame.
 * This is where we can update AI logic each tick.
 */
void AMC_EnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/**
 * Called when the controller is ending play.
 * This is where we can clean up any AI-specific logic.
 * @param EndPlayReason The reason for ending play.
 */
void AMC_EnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Stop StateTree processing
    if (StateTreeAIComponent)
    {
        StateTreeAIComponent->StopStateTree();
    }

    // Clear acquired target & owner reference
    AcquiredTarget = nullptr;
    OwningCharacter = nullptr;

    // Clear all blueprint-assignable delegates
    OnDamageStimulusEvent.Clear();
    OnSightStimulusEvent.Clear();
    OnSightStimulusForgottenEvent.Clear();
    OnHearingStimulusEvent.Clear();
    OnHearingStimulusForgottenEvent.Clear();

    // Clear perception data
    if (PerceptionComponent)
    {
        PerceptionComponent->ForgetAll();
        PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
        PerceptionComponent->OnTargetPerceptionForgotten.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

/**
 * Called when the controller possesses a pawn.
 * This is where we can initialize the AI behavior and perception.
 */
void AMC_EnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    OwningCharacter = Cast<AMC_CharacterBase>(InPawn);
    if (!OwningCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("AMC_EnemyAIController::OnPossess - InPawn is not a AMC_CharacterBase!"));
        return;
    }
}

/**
 * Called when the controller is unpossessed from a pawn.
 * This is where we can clean up or stop AI behavior.
 */
void AMC_EnemyAIController::OnUnPossess()
{
    Super::OnUnPossess();

    if (StateTreeAIComponent)
    {
        StateTreeAIComponent->StopStateTree();
    }

    OwningCharacter = nullptr; // Clear the reference to the character
    AcquiredTarget = nullptr; // Clear the acquired target
}

/**
 * Called when the perception system updates the target's perception.
 * This is where we handle the perception of the target actor.
 * @param Actor The actor that was perceived.
 * @param Stimulus The stimulus data containing information about the perception.
 */
void AMC_EnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // Validate the Actor pointer
    if (!Actor)
    {
        return;
    }

    // Check team attitude before processing
    if (GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile)
    {
        return; // Bail early, don't broadcast anything
    }

    // Static IDs for sense types (only initialized once)
    static const FAISenseID SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    static const FAISenseID HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
    static const FAISenseID DamageID = UAISense::GetSenseID(UAISense_Damage::StaticClass());

    // Determine the sense type
    FString SenseName;
    if (Stimulus.Type == SightID)
        SenseName = TEXT("Sight");
    else if (Stimulus.Type == HearingID)
        SenseName = TEXT("Hearing");
    else if (Stimulus.Type == DamageID)
        SenseName = TEXT("Damage");
    else {
        SenseName = TEXT("Unknown");
    }

    // Validate SensedActor
    AActor* SensedActor = Actor;
    if (!SensedActor)
    {
        return;
    }

    CurrentStimulusSenseType = EMC_StimulusSenseType::Unknown; // Reset to unknown before processing

    // Perception handling - use switch for clarity and perf
    if (Stimulus.Type == DamageID)
    {
        CurrentStimulusSenseType = EMC_StimulusSenseType::Damage;
        if (Stimulus.WasSuccessfullySensed())
        {
            AcquiredTarget = SensedActor; // Update acquired target on damage
            SendCombatEvent(FGameplayTag::RequestGameplayTag("MCS.StateTree.Events.Combat")); // Send event to StateTree
            OnDamageStimulusDetected(SensedActor, Stimulus);
            OnDamageStimulusEvent.Broadcast(SensedActor, Stimulus);
        }
        else
        {
            AcquiredTarget = nullptr; // Clear acquired target on forgetting
        }
    }
    else if (Stimulus.Type == SightID)
    {
        CurrentStimulusSenseType = EMC_StimulusSenseType::Sight;
        if (Stimulus.WasSuccessfullySensed())
        {
            AcquiredTarget = SensedActor; // Update acquired target on sight
            SendCombatEvent(FGameplayTag::RequestGameplayTag("MCS.StateTree.Events.Combat")); // Send event to StateTree
            OnSightStimulusDetected(SensedActor, Stimulus);
            OnSightStimulusEvent.Broadcast(SensedActor, Stimulus);
        }
        else
        {
            AcquiredTarget = nullptr; // Clear acquired target on forgetting
            OnSightStimulusForgotten(SensedActor);
            OnSightStimulusForgottenEvent.Broadcast(SensedActor);
        }
    }
    else if (Stimulus.Type == HearingID)
    {
        CurrentStimulusSenseType = EMC_StimulusSenseType::Hearing;
        if (Stimulus.WasSuccessfullySensed())
        {
            AcquiredTarget = SensedActor; // Update acquired target on hearing
            SendCombatEvent(FGameplayTag::RequestGameplayTag("MCS.StateTree.Events.Combat")); // Send event to StateTree
            OnHearingStimulusDetected(SensedActor, Stimulus);
            OnHearingStimulusEvent.Broadcast(SensedActor, Stimulus);
        }
        else
        {
            AcquiredTarget = nullptr; // Clear acquired target on forgetting
            OnHearingStimulusForgotten(SensedActor);
            OnHearingStimulusForgottenEvent.Broadcast(SensedActor);
        }
    }
}

/**
 * Called when the perception system forgets an actor.
 * This is where we handle the case where the perception of an actor is forgotten.
 * @param Actor The actor that was forgotten.
 */
void AMC_EnemyAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
    if (!Actor) return;
    if (Actor == AcquiredTarget) AcquiredTarget = nullptr; // Clear the acquired target

    OnSightStimulusForgottenEvent.Broadcast(Actor);
    OnHearingStimulusForgottenEvent.Broadcast(Actor);
    OnSightStimulusForgotten(Actor); // Broadcast sight stimulus forgotten event
    OnHearingStimulusForgotten(Actor); // Broadcast hearing stimulus forgotten event
}

/**
 * Forget a specific actor from perception.
 * @param ActorToForget The actor to forget.
 */
void AMC_EnemyAIController::ForgetPerceptionActor(AActor* ActorToForget)
{
    if (!ActorToForget)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForgetPerceptionActor: ActorToForget is nullptr."));
        return;
    }

    UAIPerceptionComponent* PC = GetPerceptionComponent();
    if (PC)
    {
        PC->ForgetActor(ActorToForget);
        UE_LOG(LogTemp, Log, TEXT("ForgetPerceptionActor: Forgot actor %s"), *ActorToForget->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ForgetPerceptionActor: PerceptionComponent is null."));
    }
}

/**
 * Forget multiple actors from perception.
 * @param ActorsToForget The array of actors to forget.
 */
void AMC_EnemyAIController::ForgetPerceptionActors(const TArray<AActor*>& ActorsToForget)
{
    UAIPerceptionComponent* PC = GetPerceptionComponent();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForgetPerceptionActors: PerceptionComponent is null."));
        return;
    }

    if (ActorsToForget.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ForgetPerceptionActors: No actors provided."));
        return;
    }

    for (AActor* Actor : ActorsToForget)
    {
        if (Actor)
        {
            PC->ForgetActor(Actor);
            UE_LOG(LogTemp, Log, TEXT("ForgetPerceptionActors: Forgot actor %s"), *Actor->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ForgetPerceptionActors: Encountered null actor, skipping."));
        }
    }
}

/**
 * Returns all actors currently sensed by the Damage sense.
 * This function retrieves all actors that have been sensed by the Damage sense.
 * @return An array of actors currently sensed by the Damage sense.
 */
TArray<AActor*> AMC_EnemyAIController::GetAllDamageSensedActors() const
{
    TArray<AActor*> SensedActors;
    if (PerceptionComponent)
    {
        PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), SensedActors);
    }

    // ✅ Filter out non-hostiles
    SensedActors = SensedActors.FilterByPredicate([ this ] (AActor* Actor)
        {
            return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
        });

    return SensedActors;
}

/**
 * Returns all actors currently sensed by the Hearing sense.
 * This function retrieves all actors that have been sensed by the Hearing sense.
 * @return An array of actors currently sensed by the Hearing sense.
 */
TArray<AActor*> AMC_EnemyAIController::GetAllHeardActors() const
{
    TArray<AActor*> SensedActors;
    if (PerceptionComponent)
    {
        PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Hearing::StaticClass(), SensedActors);
    }

    // Filter out non-hostiles
    SensedActors = SensedActors.FilterByPredicate([ this ] (AActor* Actor)
        {
            return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
        });

    return SensedActors;
}

/**
 * Returns all actors currently sensed by the Sight sense.
 * This function retrieves all actors that have been sensed by the Sight sense.
 * @return An array of actors currently sensed by the Sight sense.
 */
TArray<AActor*> AMC_EnemyAIController::GetAllSeenActors() const
{
    TArray<AActor*> SensedActors;
    if (PerceptionComponent)
    {
        PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SensedActors);
    }

    // Filter out non-hostiles
    SensedActors = SensedActors.FilterByPredicate([ this ] (AActor* Actor)
        {
            return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
        });

    return SensedActors;
}

/**
 * Report a damage event to the AI perception system.
 * This function is used to report damage events to the AI perception system.
 * @param DamagedActor The actor that was damaged.
 * @param InstigatorActor The actor that caused the damage.
 * @param DamageAmount The amount of damage inflicted.
 */
void AMC_EnemyAIController::ReportDamageEvent(AActor* DamagedActor, AActor* InstigatorActor, float DamageAmount)
{
    UWorld* World = GetWorld();
    if (!World || !DamagedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("ReportDamageEvent: Missing world or damaged actor."));
        return;
    }

    // The location of the “event” is the damaged actor’s location.
    const FVector EventLocation = DamagedActor->GetActorLocation();
    const FVector InstigatorLocation = InstigatorActor
        ? InstigatorActor->GetActorLocation()
        : EventLocation;

    // Report into UE’s perception system
    UAISense_Damage::ReportDamageEvent(
        World,
        DamagedActor,
        InstigatorActor,
        DamageAmount,
        EventLocation,
        InstigatorLocation
    );
}

/**
 * Report a noise event to the AI perception system.
 * This function is used to report noise events to the AI perception system.
 * @param NoiseInstigator The actor that made the noise.
 * @param NoiseLocation The location where the noise was made.
 * @param Loudness The loudness of the noise (default is 1.0).
 * @param MaxRange The maximum range of the noise (default is 1200.0).
 */
void AMC_EnemyAIController::ReportNoiseEvent(AActor* NoiseInstigator, FVector NoiseLocation, float Loudness, float MaxRange)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("ReportNoiseEvent: No valid World context."));
        return;
    }

    // If Instigator is null, perception will still register noise at NoiseLocation.
    UAISense_Hearing::ReportNoiseEvent(
        World,
        NoiseLocation,
        Loudness,
        NoiseInstigator,
        MaxRange
    );
}

/*
 * ========================================================================
 * Blueprint Native Event Implementations
 * These functions are now overridable in Blueprints instead of using delegates.
 * ========================================================================
 */

void AMC_EnemyAIController::OnDamageStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus)
{
    UE_LOG(LogTemp, Verbose, TEXT("[AIController] Damage stimulus detected. Strength: %.2f, Tag: %s"), Stimulus.Strength, *Stimulus.Tag.ToString());

    // Optional: handle logic or call StateTree event here
    // Example: send gameplay tag to your StateTree
    // StateTreeAIComponent->SendEvent(FGameplayTag::RequestGameplayTag(FName("MCS.StateTree.Events.Combat")));
}

void AMC_EnemyAIController::OnSightStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus)
{
    UE_LOG(LogTemp, Verbose, TEXT("[AIController] Sight stimulus detected at location: %s"), *Stimulus.StimulusLocation.ToString());
}

void AMC_EnemyAIController::OnSightStimulusForgotten_Implementation(AActor* Actor)
{
    UE_LOG(LogTemp, Verbose, TEXT("[AIController] Sight stimulus forgotten for actor: %s"), *GetNameSafe(Actor));
}

void AMC_EnemyAIController::OnHearingStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus)
{
    UE_LOG(LogTemp, Verbose, TEXT("[AIController] Hearing stimulus detected at location: %s"), *Stimulus.StimulusLocation.ToString());
}

void AMC_EnemyAIController::OnHearingStimulusForgotten_Implementation(AActor* Actor)
{
    UE_LOG(LogTemp, Verbose, TEXT("[AIController] Hearing stimulus forgotten for actor: %s"), *GetNameSafe(Actor));
}
