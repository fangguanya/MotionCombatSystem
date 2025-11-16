/*
 * ========================================================================
 * Copyright © 2025 God's Studio
 * All Rights Reserved.
 *
 * Free for all to use, copy, and distribute. I hope you learn from this as I learned creating it.
 * =============================================================================
 *
 * Project: Motion Combat Sample
 * Author: Christopher D. Parker
 * Date: 11-10-2025
 * =============================================================================
 * MC_EnemyAIController.h
 * Custom AI Controller for Enemy Characters (StateTree-ready)
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Touch.h"
#include "Perception/AISenseConfig_Prediction.h"
#include "GenericTeamAgentInterface.h"
#include <Characters/MC_CharacterBase.h>
#include <Enums/MC_StimulusSenseType.h>
#include <Enums/MC_AIStateEnums.h>
#include <Components/MC_StateTreeAIComponent.h>
#include "MC_EnemyAIController.generated.h"


 // Delegate for perception updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCStimulusEventSignature, AActor*, Actor, const FAIStimulus&, Stimulus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMCStimulusForgottenSignature, AActor*, Actor);


/**
 * Custom AI Controller for Enemy Characters
 * This controller manages the AI behavior for enemy characters using State Trees.
*/
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent, DisplayName = "Motion Combat Enemy AI Controller"))
class MOTIONCOMBAT_API AMC_EnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    /*
     * Functions
     */

    // Constructor
    AMC_EnemyAIController();

    // Returns the State Tree AI Component for this character
    // This component handles the AI behavior for the enemy character.
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI", meta = (DisplayName = "Get State Tree AI Component"))
    UMC_StateTreeAIComponent* GetStateTreeAIComponent() const { return StateTreeAIComponent; };

    /**
     * Gets the current stimulus sense type.
     * @return The current stimulus sense type.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI", meta = (DisplayName = "Get Current Stimulus Sense Type"))
    EMC_StimulusSenseType GetCurrentStimulusSenseType() const { return CurrentStimulusSenseType; };

    /**
     * Checks if the stimulus is from a hostile actor.
     * @param Actor The actor to check.
     * @return True if the actor is hostile, false otherwise.
     */
    bool IsStimulusFromHostile(AActor* Actor) const
    {
        return Actor && GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
    }

    /**
     * Checks if there is an acquired target.
     * @return True if there is an acquired target, false otherwise.
     */
    bool HasAcquiredTarget() const
    {
        return AcquiredTarget != nullptr;
    }

    /**
     * Gets the owning character.
     * @return The owning character.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI", meta = (DisplayName = "Get Owning Character"))
    AMC_CharacterBase* GetOwningCharacter() const
    {
        return OwningCharacter;
    }

    /**
     * Forget a specific actor from perception.
     * @param ActorToForget The actor to forget.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI")
    void ForgetPerceptionActor(AActor* ActorToForget);

    /**
     * Forget multiple actors from perception.
     * @param ActorsToForget The array of actors to forget.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI")
    void ForgetPerceptionActors(const TArray<AActor*>& ActorsToForget);

    /**
     * Returns all actors currently sensed by the Damage sense.
     * This function retrieves all actors that have been sensed by the Damage sense.
     * @return An array of actors currently sensed by the Damage sense.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI")
    TArray<AActor*> GetAllDamageSensedActors() const;

    /**
     * Returns all actors currently sensed by the Hearing sense.
     * This function retrieves all actors that have been sensed by the Hearing sense.
     * @return An array of actors currently sensed by the Hearing sense.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI")
    TArray<AActor*> GetAllHeardActors() const;

    /**
     * Returns all actors currently sensed by the Sight sense.
     * This function retrieves all actors that have been sensed by the Sight sense.
     * @return An array of actors currently sensed by the Sight sense.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI")
    TArray<AActor*> GetAllSeenActors() const;

    /**
     * Report a damage event to the AI perception system.
     * This function is used to report damage events to the AI perception system.
     * @param DamagedActor The actor that was damaged.
     * @param InstigatorActor The actor that caused the damage.
     * @param DamageAmount The amount of damage inflicted.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Perception")
    void ReportDamageEvent(AActor* DamagedActor, AActor* InstigatorActor, float DamageAmount);

    /**
     * Report a noise event to the AI perception system.
     * This function is used to report noise events to the AI perception system.
     * @param NoiseInstigator The actor that made the noise.
     * @param NoiseLocation The location where the noise was made.
     * @param Loudness The loudness of the noise (default is 1.0).
     * @param MaxRange The maximum range of the noise (default is 1200.0).
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|Perception")
    void ReportNoiseEvent(AActor* NoiseInstigator, FVector NoiseLocation, float Loudness = 1.f, float MaxRange = 1200.f);

    /**
     * Sends a gameplay event to the StateTree AI Component.
     * This is used by perception callbacks or any other system that wants to notify the AI.
     */
    UFUNCTION(BlueprintCallable, Category = "Motion Combat|AI|StateTree", meta = (DisplayName = "Send StateTree Event"))
    void SendCombatEvent(const FGameplayTag& EventTag)
    {
        if (!StateTreeAIComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] SendCombatEvent FAILED – No StateTreeAIComponent attached."));
            return;
        }

        if (!EventTag.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] SendCombatEvent FAILED – Invalid GameplayTag."));
            return;
        }

        UE_LOG(LogTemp, Log, TEXT("[EnemyAIController] Sending StateTree Event: %s"), *EventTag.ToString());

        StateTreeAIComponent->SendStateTreeEvent(EventTag);
    }

    /*
     * Native Events
     */

    /**
     * Called when a damage stimulus is detected.
     * @param SensedActor The actor associated with the stimulus.
     * @param Stimulus The stimulus data containing perception info.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|AI|Events", meta = (DisplayName = "On Damage Stimulus Detected"))
    void OnDamageStimulusDetected(AActor* SensedActor, const FAIStimulus& Stimulus);
    virtual void OnDamageStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus);

    /**
     * Called when a sight stimulus is detected.
     * @param SensedActor The actor associated with the stimulus.
     * @param Stimulus The stimulus data containing perception info.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|AI|Events", meta = (DisplayName = "On Sight Stimulus Detected"))
    void OnSightStimulusDetected(AActor* SensedActor, const FAIStimulus& Stimulus);
    virtual void OnSightStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus);

    /**
     * Called when a sight stimulus is forgotten.
     * @param Actor The actor that was forgotten.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|AI|Events", meta = (DisplayName = "On Sight Stimulus Forgotten"))
    void OnSightStimulusForgotten(AActor* Actor);
    virtual void OnSightStimulusForgotten_Implementation(AActor* Actor);

    /**
     * Called when a hearing stimulus is detected.
     * @param SensedActor The actor associated with the stimulus.
     * @param Stimulus The stimulus data containing perception info.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|AI|Events", meta = (DisplayName = "On Hearing Stimulus Detected"))
    void OnHearingStimulusDetected(AActor* SensedActor, const FAIStimulus& Stimulus);
    virtual void OnHearingStimulusDetected_Implementation(AActor* SensedActor, const FAIStimulus& Stimulus);

    /**
     * Called when a hearing stimulus is forgotten.
     * @param Actor The actor that was forgotten.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|AI|Events", meta = (DisplayName = "On Hearing Stimulus Forgotten"))

    void OnHearingStimulusForgotten(AActor* Actor);
    virtual void OnHearingStimulusForgotten_Implementation(AActor* Actor);

    /*
     * Properties
     */

    /**
     * The current acquired target from perception events
     * Placed here for easy access in Blueprints and can be shared in State Trees.
     * This is the actor that the AI is currently focused on due to perception.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Output, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AActor> AcquiredTarget = nullptr;

    /*
     * Assignable Events
     */

    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|AI|Perception")
    FMCStimulusEventSignature OnDamageStimulusEvent;

    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|AI|Perception")
    FMCStimulusEventSignature OnSightStimulusEvent;

    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|AI|Perception")
    FMCStimulusForgottenSignature OnSightStimulusForgottenEvent;

    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|AI|Perception")
    FMCStimulusEventSignature OnHearingStimulusEvent;

    UPROPERTY(BlueprintAssignable, Category = "Motion Combat|AI|Perception")
    FMCStimulusForgottenSignature OnHearingStimulusForgottenEvent;

protected:

    /*
     * Functions
     */

    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

    /** Called every frame. */
    virtual void Tick(float DeltaTime) override;

    /** Called when the controller is ending play */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Called when the controller possesses a pawn */
    virtual void OnPossess(APawn* InPawn) override;

    /** Called when the controller is unpossessed */
    virtual void OnUnPossess() override;

    // Perception updated callback
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnTargetPerceptionForgotten(AActor* Actor);

private:
    
    /*
     * Properties
     */

    // State tree AI component for managing enemy AI behavior
    // This component handles the AI behavior for the enemy character.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Combat|AI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMC_StateTreeAIComponent> StateTreeAIComponent;

    /** Enemy character this controller is managing */
    TObjectPtr<AMC_CharacterBase> OwningCharacter = nullptr;

    // Sight configuration object
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    // Hearing config
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // Damage config
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Damage> DamageConfig;

    // Prediction config
    UPROPERTY()
    TObjectPtr<UAISenseConfig_Prediction> PredictionConfig;

    UPROPERTY()
    EMC_StimulusSenseType CurrentStimulusSenseType = EMC_StimulusSenseType::Unknown;
};
