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
 * MC_AINodeBase.cpp
 * Implementation of the base AI Node class.
 */

#include "AI/Nodes/MC_AINodeBase.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UMC_AINodeBase::UMC_AINodeBase()
{
    bIsRunning = false;
}

void UMC_AINodeBase::FinishNode(bool bSuccess)
{
    if (!bIsRunning)
        return;

    bIsRunning = false;
    OnNodeFinished.Broadcast(bSuccess);
}

void UMC_AINodeBase::SetRunning(bool bRunningIn)
{
    bIsRunning = bRunningIn;
}
