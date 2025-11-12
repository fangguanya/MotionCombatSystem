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
 * Enum defining the types of stimulus senses used in MC_StimulusSenseType.
 */

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType, meta = (DisplayName = "Motion Combat Stimulus Sense"))
enum class EMC_StimulusSenseType : uint8
{
    Unknown UMETA(DisplayName = "Unknown"),
    Sight   UMETA(DisplayName = "Sight"),
    Hearing UMETA(DisplayName = "Hearing"),
    Damage  UMETA(DisplayName = "Damage")
};