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
 * Date: 11-13-2025
 * =============================================================================
 * A StateTree task that takes a heterogenous array of parameters (float, int, string, name, text)
 * and displays a formatted debug string at an actor’s location (or the world origin).
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "DrawDebugHelpers.h"
#include "StateTreeExecutionContext.h"
#include "MC_DebugFormattedText.generated.h"


UENUM(BlueprintType, Category = "Motion Combat|State Tree|Tasks",
    meta = (DisplayName = "Debug Parameter Type",
        Description = "Enumeration of parameter types for debug formatted text.",
        ToolTip = "Enumeration of parameter types for debug formatted text."))
    enum class EMC_DebugParamType : uint8
{
    Float   UMETA(DisplayName = "Float"),
    Int     UMETA(DisplayName = "Int"),
    String  UMETA(DisplayName = "String"),
    Name    UMETA(DisplayName = "Name"),
    Text    UMETA(DisplayName = "Text"),
    Bool    UMETA(DisplayName = "Bool"),
    Vector  UMETA(DisplayName = "Vector")
};

USTRUCT(BlueprintType, Category = "Motion Combat|State Tree|Tasks",
    meta = (DisplayName = "Debug Format Parameter",
        Description = "Structure representing a single parameter for formatted debug text.",
        ToolTip = "Structure representing a single parameter for formatted debug text."))
    struct FMC_DebugFormatParam
{
    GENERATED_BODY()

    /** The placeholder key in your FormatText, e.g. "Health" for "{Health}" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Key"))
    FName Key;

    /** Which value slot to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Type"))
    EMC_DebugParamType Type = EMC_DebugParamType::Float;

    /** Only valid if Type==Float */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Float Value", EditCondition = "Type == EMC_DebugParamType::Float"))
    float FloatValue = 0.f;

    /** Only valid if Type==Int */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Int Value", EditCondition = "Type == EMC_DebugParamType::Int"))
    int32 IntValue = 0;

    /** Only valid if Type==String */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "String Value", EditCondition = "Type == EMC_DebugParamType::String"))
    FString StringValue;

    /** Only valid if Type==Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Name Value", EditCondition = "Type == EMC_DebugParamType::Name"))
    FName NameValue;

    /** Only valid if Type==Text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Text Value", EditCondition = "Type == EMC_DebugParamType::Text"))
    FText TextValue;

    /** Only valid if Type==Bool */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (DisplayName = "Bool Value", EditCondition = "Type == EMC_DebugParamType::Bool"))
    bool BoolValue;

    /** Only valid if Type==Vector */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input,
        meta = (DisplayName = "Vector Value", EditCondition = "Type == EMC_DebugParamType::Vector"))
    FVector VectorValue = FVector::ZeroVector;
};

/**
 * Displays a formatted debug string using a printf-style format.
 * Example FormatString: "Health: %.2f"
 * Parameter will be substituted for the “%f”.
 */
UCLASS(meta = (DisplayName = "Debug Formatted Text", Category = "Motion Combat|State Tree|Tasks", Description = "Displays a formatted debug text at an actor's location (or origin)."))
class MOTIONCOMBAT_API UMC_DebugFormattedText : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:

    /**
     * Printf-style format.
     * E.g. TEXT("Health: %.2f / %.2f")
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
    FText FormatText = NSLOCTEXT("PGAS", "DebugFmt", "Type Something {Value}");

    /** Single float parameter to be inserted into FormatString */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
    TArray<FMC_DebugFormatParam> Params;

    /**
     * Optional actor to attach the debug text to.
     *   If null, world origin (0,0,0) is used.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (Input, DisplayName = "Actor Context"))
    TObjectPtr<AActor> Actor = nullptr;

    /** If true, also print the message via AddOnScreenDebugMessage in black */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task", meta = (Input, DisplayName = "Print To Screen"))
    bool PrintToScreen = false;

#if WITH_EDITOR

    /**
     * Gets the description of the condition for editor display.
     * @param ID The unique identifier for the node.
     * @param InstanceDataView The data view for the instance.
     * @param BindingLookup The binding lookup for the state tree.
     * @param Formatting The formatting options for the description.
     * @return The formatted description text.
    */
    virtual FText GetDescription(
        const FGuid& ID,
        FStateTreeDataView InstanceDataView,
        const IStateTreeBindingLookup& BindingLookup,
        EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const {
        return NSLOCTEXT(
            "MotionCombat",
            "DebugFormattedTextDesc",
            "Display Formatted Debug Text at Actor Location"
        );
    };

    virtual FName GetIconName() const {
        return FName("Info_Circle");
    };

#endif

protected:

    /**
     * This function will find a random reachable location around the actor within the specified radius.
     * @param Context The execution context for the state tree.
     * @param Transition The transition result containing information about the state change.
     */
    EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
    {
        // Resolve world
        UWorld* World = Actor
            ? Actor->GetWorld()
            : Context.GetWorld();

        if (!World)
        {
            UE_LOG(LogTemp, Warning, TEXT("UPGAS_DebugFormattedText: No valid World!"));
            return EStateTreeRunStatus::Failed;
        }

        // Build named arguments
        FFormatNamedArguments Args;
        for (const FMC_DebugFormatParam& P : Params)
        {
            const FString KeyString = P.Key.ToString();

            switch (P.Type)
            {
                case EMC_DebugParamType::Float:
                    Args.Add(KeyString, FFormatArgumentValue(P.FloatValue));
                    break;

                case EMC_DebugParamType::Int:
                    Args.Add(KeyString, FFormatArgumentValue(P.IntValue));
                    break;

                case EMC_DebugParamType::String:
                    // Convert FString → FText → FFormatArgumentValue
                    Args.Add(KeyString, FFormatArgumentValue(FText::FromString(P.StringValue)));
                    break;

                case EMC_DebugParamType::Name:
                    // Convert FName → FText → FFormatArgumentValue
                    Args.Add(KeyString, FFormatArgumentValue(FText::FromName(P.NameValue)));
                    break;

                case EMC_DebugParamType::Text:
                    // Already an FText
                    Args.Add(KeyString, FFormatArgumentValue(P.TextValue));
                    break;

                case EMC_DebugParamType::Bool:
                    Args.Add(KeyString, FFormatArgumentValue(P.BoolValue));
                    break;

                case EMC_DebugParamType::Vector:
                    {
                        const FString VecStr = P.VectorValue.ToString();
                        Args.Add(KeyString, FFormatArgumentValue(FText::FromString(VecStr)));
                        break;
                    }
            }
        }

        // Format the text
        const FText FormattedText = FText::Format(FormatText, Args);
        const FString Message = FormattedText.ToString();

        // Pick a draw location
        const FVector DrawLoc = Actor
            ? Actor->GetActorLocation()
            : FVector::ZeroVector;

        // Draw (Duration=0 → one frame)
        DrawDebugString(World, DrawLoc, Message, Actor.Get(), FColor::White, /*Duration=*/2.f, /*bDrawShadow=*/true);

        // Optionally print to screen (black, two seconds)
        if (PrintToScreen && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                /*Key=*/-1,
                /*TimeToDisplay=*/2.0f,
                /*Color=*/FColor::Black,
                /*Message=*/Message
            );
        }

        return EStateTreeRunStatus::Succeeded;
    }
};
