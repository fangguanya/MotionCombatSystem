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
 * Date: 10-27-2025
 * =============================================================================
 * MCS_CombatEventBus.cpp
 * Global event bus for Motion Combat System – enables decoupled communication
 * between combat components (Core, Defense, HitReaction, etc.)
 */

#include "Events/MCS_CombatEventBus.h"
#include "Engine/World.h"

TMap<TWeakObjectPtr<UWorld>, TObjectPtr<UMCS_CombatEventBus>> UMCS_CombatEventBus::BusInstances;

/**
 * Returns or creates the global Combat Event Bus for this world
 * @param World The UWorld context
 * @return The Combat Event Bus instance for the given world
 */
UMCS_CombatEventBus* UMCS_CombatEventBus::Get(UWorld* World)
{
    if (!World) return nullptr;

    if (TObjectPtr<UMCS_CombatEventBus>* Existing = BusInstances.Find(World))
    {
        return *Existing;
    }

    // Create new instance
    UMCS_CombatEventBus* NewBus = NewObject<UMCS_CombatEventBus>(World, UMCS_CombatEventBus::StaticClass());
    BusInstances.Add(World, NewBus);

    UE_LOG(LogTemp, Log, TEXT("[MCS_EventBus] Created new Combat Event Bus for world: %s"), *World->GetName());
    return NewBus;
}

/**
 * Blueprint-accessible version to get the Combat Event Bus for the given world context
 * @param WorldContextObject The context object to derive the UWorld from
 */
UMCS_CombatEventBus* UMCS_CombatEventBus::GetCombatEventBus(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;

    return Get(WorldContextObject->GetWorld());
}

/**
 * Cleans up the event bus instance when the object is being destroyed
 */
void UMCS_CombatEventBus::BeginDestroy()
{
    BusInstances.Remove(GetWorld());

    Super::BeginDestroy();
}
