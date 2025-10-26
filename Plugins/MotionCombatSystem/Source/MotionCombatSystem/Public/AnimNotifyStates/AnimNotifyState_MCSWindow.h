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
 * Date: 10-26-2025
 * =============================================================================
 * AnimNotifyState_MCSWindow.h
 * A multi-purpose Anim Notify State used by the Motion Combat System.
 * It can represent hitbox windows, combo windows, parry windows, etc.,
 * based on the designer-selected EventType enum.
 */

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include <Structs/MCS_AttackHitbox.h>
#include "AnimNotifyState_MCSWindow.generated.h"


 /**
 * Enum describing what type of MCS event this notify represents.
 */
UENUM(BlueprintType, Category = "MotionCombatSystem", Meta = (DisplayName = "Motion Combat Anim Event Type"))
enum class EMCS_AnimEventType : uint8
{
    None            UMETA(DisplayName = "None"),
    HitboxWindow    UMETA(DisplayName = "Hitbox Window"),
    ComboWindow     UMETA(DisplayName = "Combo Window"),
    AttackStart     UMETA(DisplayName = "Attack Start"),
    DefenseWindow   UMETA(DisplayName = "Defense Window"),
    ParryWindow     UMETA(DisplayName = "Parry Window"),
    Custom          UMETA(DisplayName = "Custom (User Defined)")
};


/*
 * Delegates
 */

// Delegate called when the notify begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMCSNotifyBegin, EMCS_AnimEventType, EventType, UAnimNotifyState_MCSWindow*, NotifyInstance);

// Delegate called every tick during the notify window
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMCSNotifyTick, EMCS_AnimEventType, EventType, UAnimNotifyState_MCSWindow*, NotifyInstance);

// Delegate called when the notify ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMCSNotifyEnd, EMCS_AnimEventType, EventType, UAnimNotifyState_MCSWindow*, NotifyInstance);


/**
 * Multi-purpose MCS Anim Notify State that handles multiple event types within the Motion Combat System. Designers choose the event type in
 * the animation timeline and bind behavior via Blueprint or C++.
 */
UCLASS(Blueprintable, ClassGroup = (MotionCombatSystem), meta = (DisplayName = "Motion Combat Notify Window"))
class MOTIONCOMBATSYSTEM_API UAnimNotifyState_MCSWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    /*
     * Properties
     */

     /** Optional unique ID for this notify */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Unique ID"))
    FName Id = NAME_None;
    
    /** Type of Motion Combat System event this notify represents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Event Type"))
    EMCS_AnimEventType EventType = EMCS_AnimEventType::None;

    /** Name of the event (for identification purposes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Event Name"))
    FString Name;

    /** Optional debug label for clarity in animation timelines */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Debug Label"))
    FName DebugLabel = NAME_None;

    /** Enable debug visualization/logging for this window */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Debug"))
    bool bDebug = false;

    /** Optional gameplay tag for external systems (GAS, AI, etc.) to identify this event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS|Tags", Meta = (DisplayName = "Event Tag"))
    FGameplayTag EventTag;

    /** Hitbox configuration for this notify window (designer-defined). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MCS", Meta = (DisplayName = "Hitbox"))
    FMCS_AttackHitbox Hitbox;

    /** Broadcast when notify begins */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events")
    FOnMCSNotifyBegin OnNotifyBegin;

    /** Broadcast every tick during notify window */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events")
    FOnMCSNotifyTick OnNotifyTick;

    /** Broadcast when notify ends */
    UPROPERTY(BlueprintAssignable, Category = "MCS|Events")
    FOnMCSNotifyEnd OnNotifyEnd;

    /*
     * Blueprint Events
     * These allow designers to respond directly to notify events without binding delegates.
     */

     /** Called when the notify begins */
    UFUNCTION(BlueprintImplementableEvent, Category = "MCS|Events")
    void OnMCSNotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, EMCS_AnimEventType InEventType) const;

    /** Called every tick during the notify window */
    UFUNCTION(BlueprintImplementableEvent, Category = "MCS|Events")
    void OnMCSNotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, EMCS_AnimEventType InEventType, float DeltaTime) const;

    /** Called when the notify ends */
    UFUNCTION(BlueprintImplementableEvent, Category = "MCS|Events")
    void OnMCSNotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, EMCS_AnimEventType InEventType) const;

    /*
     * Functions
     */

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;

        OnNotifyBegin.Broadcast(EventType, this);
        OnMCSNotifyBegin(MeshComp, Animation, EventType);

        if (bDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("[MCSNotify] %s Begin (%s) | Tag: %s"), *DebugLabel.ToString(), *UEnum::GetValueAsString(EventType), *EventTag.ToString());
        }
    }

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;

        OnNotifyTick.Broadcast(EventType, this);
        OnMCSNotifyTick(MeshComp, Animation, EventType, FrameDeltaTime);

        if (bDebug)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("[MCSNotify] %s Tick (%s) Δ=%.3fs"), *DebugLabel.ToString(), *UEnum::GetValueAsString(EventType), FrameDeltaTime);
        }
    }

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;

        OnNotifyEnd.Broadcast(EventType, this);
        OnMCSNotifyEnd(MeshComp, Animation, EventType);

        if (bDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("[MCSNotify] %s End (%s)"), *DebugLabel.ToString(), *UEnum::GetValueAsString(EventType));
        }
    }

    /** Returns a clean, designer-friendly name in animation editor */
    virtual FString GetNotifyName_Implementation() const override
    {
        // If designer gave a custom name, use it
        if (!Name.IsEmpty())
        {
            return Name;
        }

        // Otherwise, fall back to enum name
        return UEnum::GetDisplayValueAsText(EventType).ToString();
    }

#if WITH_EDITOR
    /** Returns a unique color for this notify based on event type (for easy timeline identification) */
    virtual FLinearColor GetEditorColor() override
    {
        switch (EventType)
        {
            case EMCS_AnimEventType::HitboxWindow:   return FLinearColor::Red;
            case EMCS_AnimEventType::ComboWindow:    return FLinearColor::Blue;
            case EMCS_AnimEventType::ParryWindow:    return FLinearColor::Green;
            case EMCS_AnimEventType::DefenseWindow:  return FLinearColor::Yellow;
            case EMCS_AnimEventType::AttackStart:    return FLinearColor::Gray;
            default:                                 return FLinearColor::Black;
        }
    }
#endif
};
