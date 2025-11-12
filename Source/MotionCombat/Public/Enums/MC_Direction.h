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
 * Enum defining Direction for character movement.
 */

#pragma once

#include "MC_Direction.generated.h"

UENUM(BlueprintType, meta=(DisplayName = "Motion Combat Character Movement Direction", ShortTooltip = "Direction enum for character movement"))
enum class EMC_Direction : uint8
{
    Forward  UMETA(DisplayName = "Forward"),
    Backward UMETA(DisplayName = "Backward"),
    Left     UMETA(DisplayName = "Left"),
    Right    UMETA(DisplayName = "Right")
};
