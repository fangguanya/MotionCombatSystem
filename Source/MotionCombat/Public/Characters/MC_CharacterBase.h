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
 * MC_CharacterBase.h
 * Base character class for Motion Combat.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StateTreeReference.h"
#include "GenericTeamAgentInterface.h"
#include "MC_CharacterBase.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (MotionCombatSystem),
	meta = (DisplayName = "Motion Combat Character Base", ShortTooltip = "Base character class for Motion Combat.")
)
class MOTIONCOMBAT_API AMC_CharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMC_CharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Checks if the character is currently attacking.
	 * This function is used to determine if the character is in an attacking state.
	 * @return bool True if the character is attacking, false otherwise.
	*/
	UFUNCTION(BlueprintPure, Category = "Player|Combat", meta = (DisplayName = "Is Currently Attacking", HideSelfPin = "true", ReturnDisplayName = "Is Attacking", ToolTip = "Checks if the character is currently attacking."))
	bool IsAttacking() const
	{
		return bIsAttacking;
	}

	/**
	 * Sets the attacking state of the character.
	 * This function is used to set whether the character is currently attacking or not.
	 * @param bAttacking True if the character is attacking, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Combat", meta = (DisplayName = "Set Is Attacking", ToolTip = "Sets the attacking state of the character."))
	void SetIsAttacking(bool bAttacking)
	{
		bIsAttacking = bAttacking;
	}

	// Returns the State Tree asset for this character.
	// This is used for AI behavior logic.
	UFUNCTION(BlueprintPure, Category = "Player|AI", meta = (DisplayName = "Get State Tree", ReturnDisplayName = "State Tree"))
	UStateTree* GetStateTree() { return StateTree; }

	/** IGenericTeamAgentInterface override */
	virtual FGenericTeamId GetGenericTeamId() const override
	{
		return FGenericTeamId(TeamID);
	}

	/** Optional: Allow setting team in C++/Blueprint */
	UFUNCTION(BlueprintCallable, Category = "AI|Team")
	void SetTeamID(uint8 NewID)
	{
		TeamID = NewID;
	}

	/*
	* Properties
	*/
	
	/** Team identifier for AI perception & hostility logic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Team")
	uint8 TeamID = 1;  // Default = Player team. Enemies can override in BP to 2, 3, etc.

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	/*
	* Properties
	*/

	// Flag to check if the character is currently attacking
	bool bIsAttacking = false;

	// Reference to the StateTree asset for AI logic
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|AI", meta = (AllowPrivateAccess = "true"))
	UStateTree* StateTree;
};
