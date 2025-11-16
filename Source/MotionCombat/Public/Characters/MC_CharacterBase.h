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
#include <Components/MC_HealthComponent.h>
#include "MC_CharacterBase.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (MotionCombat),
	meta = (DisplayName = "Motion Combat Character Base",
		ShortTooltip = "Base character class for Motion Combat.")
)
class MOTIONCOMBAT_API AMC_CharacterBase : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	/*
	* Functions
	*/
	
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
	UFUNCTION(BlueprintCallable, Category = "Player|Combat",
		meta = (DisplayName = "Set Is Attacking", ToolTip = "Sets the attacking state of the character."))
	void SetIsAttacking(bool bAttacking)
	{
		bIsAttacking = bAttacking;
	}

	/**
	 * Checks if the character is currently defending.
	 * This function is used to determine if the character is in a defending state.
	 * @return bool True if the character is defending, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Player|Combat",
		meta = (DisplayName = "Is Currently Defending", HideSelfPin = "true", ReturnDisplayName = "Is Defending", ToolTip = "Checks if the character is currently defending."))
	bool IsDefending() const
	{
		return bIsDefending;
	}

	/**
	 * Sets the defending state of the character.
	 * This function is used to set whether the character is currently defending or not.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Combat",
		meta = (DisplayName = "Set Is Defending", ToolTip = "Sets the defending state of the character."))
	void SetIsDefending(bool bDefending)
	{
		bIsDefending = bDefending;
	}

	/**
	 * Checks if the character is dead.
	 * This function is used to determine if the character is deceased.
	 * @return bool True if the character is dead, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Player|Combat",
		meta = (DisplayName = "Is Currently Dead", HideSelfPin = "true", ReturnDisplayName = "Is Dead", ToolTip = "Checks if the character is currently dead."))
	bool IsDead() const
	{
		return bIsDead;
	}

	/**
	 * Sets the deceased state of the character.
	 * This function is used to set whether the character is dead or alive.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player|Combat",
		meta = (DisplayName = "Set Is Dead", ToolTip = "Sets the deceased state of the character."))
	void SetIsDead(bool bDead)
	{
		bIsDead = bDead;
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

	// Get the current health of the character
	UFUNCTION(BlueprintPure, Category = "Motion Combat|Health", meta = (DisplayName = "Get Current Health", ReturnDisplayName = "Current Health"))
	float GetHealth() const
	{
		if (HealthComponent)
		{
			return HealthComponent->CurrentHealth;
		}
		return 0.f;
	}

	/*
	 * Event Handlers
	 */

	/** Called when character health is updated */
	UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|Health")
	void OnCharacterHealthChanged(float NewHealth, float MaxHealth);
	virtual void OnCharacterHealthChanged_Implementation(float NewHealth, float MaxHealth);

	/** Called when character takes damage */
	UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|Health")
	void OnCharacterDamageTaken(float DamageAmount, float NewHealth, AActor* InstigatorActor);
	virtual void OnCharacterDamageTaken_Implementation(float DamageAmount, float NewHealth, AActor* InstigatorActor);

	/** Called when character dies */
	UFUNCTION(BlueprintNativeEvent, Category = "Motion Combat|Health")
	void OnCharacterDeath(AActor* DeadActor);
	virtual void OnCharacterDeath_Implementation(AActor* DeadActor);

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

	bool bIsDefending = false;

	bool bIsDead = false;

	// Reference to the StateTree asset for AI logic
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|AI", meta = (AllowPrivateAccess = "true"))
	UStateTree* StateTree;

	// Reference to the health component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player|Health", meta = (AllowPrivateAccess = "true"))
	UMC_HealthComponent* HealthComponent;
};
