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
 * MC_AINodeLibrary.cpp
 * Implements Blueprint helper nodes for AI behavior flow.
 */

#include <AI/Libraries/MC_AINodeLibrary.h>
#include <Characters/MC_CharacterBase.h>
#include <Controllers/MC_EnemyAIController.h>
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Components/MCS_CombatCoreComponent.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "LatentActions.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"


void UMC_AINodeLibrary::StartAINode(UObject* WorldContextObject, UMC_AINodeBase* Node, AAIController* Controller, APawn* Pawn)
{
    if (!Node || !Controller || !Pawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AINodeLibrary] StartAINode: Invalid input(s)."));
        return;
    }

    Node->SetRunning(true);
    Node->OnEnterNode(Controller, Pawn);

    // Register node with the controller to tick it
    if (AMC_EnemyAIController* EnemyController = Cast<AMC_EnemyAIController>(Controller))
    {
        EnemyController->SetActiveAINode(Node);
    }
}

void UMC_AINodeLibrary::WaitForSeconds(UObject* WorldContextObject, FLatentActionInfo LatentInfo, float Duration)
{
    if (!WorldContextObject)
        return;

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World)
        return;

    struct FWaitForSecondsAction : public FPendingLatentAction
    {
        float TimeRemaining;
        FLatentActionInfo Info;

        FWaitForSecondsAction(float InDuration, const FLatentActionInfo& InInfo)
            : TimeRemaining(InDuration), Info(InInfo)
        {
        }

        virtual void UpdateOperation(FLatentResponse& Response) override
        {
            TimeRemaining -= Response.ElapsedTime();
            Response.FinishAndTriggerIf(TimeRemaining <= 0.0f, Info.ExecutionFunction, Info.Linkage, Info.CallbackTarget);
        }
    };

    FLatentActionManager& LatentManager = World->GetLatentActionManager();

    // Prevent duplicate latent actions for this object
    if (LatentManager.FindExistingAction<FWaitForSecondsAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    {
        LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FWaitForSecondsAction(Duration, LatentInfo));
    }
}

bool UMC_AINodeLibrary::MoveToTarget(UObject* WorldContextObject, AAIController* Controller, AActor* TargetActor, float AcceptanceRadius)
{
    if (!Controller || !TargetActor)
        return false;

    // Request the move
    const EPathFollowingRequestResult::Type Result =
        Controller->MoveToActor(TargetActor, AcceptanceRadius, true, true, true, 0, true);

    // In UE 5.5+ “RequestSuccessful” lives under EPathFollowingRequestResult::Type
    const bool bSuccess =
        (Result == EPathFollowingRequestResult::Type::RequestSuccessful ||
            Result == EPathFollowingRequestResult::Type::AlreadyAtGoal);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[AINodeLibrary] Moving towards target: %s"), *TargetActor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AINodeLibrary] MoveToTarget failed for %s"), *TargetActor->GetName());
    }

    return bSuccess;
}

bool UMC_AINodeLibrary::AttackTarget(UObject* WorldContextObject, AActor* InstigatorActor, AActor* TargetActor)
{
    if (!InstigatorActor || !TargetActor)
        return false;

    UActorComponent* CoreComp = InstigatorActor->FindComponentByClass(UMCS_CombatCoreComponent::StaticClass());
    UMCS_CombatCoreComponent* CombatCore = Cast<UMCS_CombatCoreComponent>(CoreComp);

    if (!CombatCore)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AINodeLibrary] AttackTarget: Missing CombatCoreComponent on %s"), *InstigatorActor->GetName());
        return false;
    }

    // if (CombatCore->SelectAttack())
    // {
    //     CombatCore->ExecuteAttack();
    //     return true;
    // }

    return false;
}

void UMC_AINodeLibrary::ChangeAIState(UObject* WorldContextObject, AAIController* Controller, EMC_AIState NewState)
{
    if (!Controller)
        return;

    FString StateName = UEnum::GetValueAsString(NewState);
    UE_LOG(LogTemp, Log, TEXT("[AINodeLibrary] Changing AI state to: %s"), *StateName);
}

bool UMC_AINodeLibrary::FindRandomPatrolLocation(UObject* WorldContextObject, const FVector& Origin, float Radius, FVector& OutLocation)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World)
        return false;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
        return false;

    FNavLocation NavLoc;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLoc);
    if (bFound)
    {
        OutLocation = NavLoc.Location;
    }
    return bFound;
}

bool UMC_AINodeLibrary::MoveToRandomLocation(UObject* WorldContextObject, AAIController* Controller, float Radius, float AcceptanceRadius)
{
    if (!Controller)
        return false;

    APawn* Pawn = Controller->GetPawn();
    if (!Pawn)
        return false;

    FVector Origin = Pawn->GetActorLocation();
    FVector RandomLocation;
    if (!FindRandomPatrolLocation(WorldContextObject, Origin, Radius, RandomLocation))
    {
        UE_LOG(LogTemp, Warning, TEXT("[AINodeLibrary] Failed to find random patrol location."));
        return false;
    }

    EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(RandomLocation, AcceptanceRadius, true, true);
    const bool bSuccess = (Result == EPathFollowingRequestResult::RequestSuccessful);

#if WITH_EDITOR
    if (bSuccess)
        DrawDebugSphere(Controller->GetWorld(), RandomLocation, 30.f, 12, FColor::Yellow, false, 3.f);
#endif

    return bSuccess;
}

void UMC_AINodeLibrary::FaceTarget(UObject* WorldContextObject, APawn* Pawn, AActor* TargetActor, float InterpSpeed)
{
    if (!Pawn || !TargetActor)
        return;

    const FVector PawnLocation = Pawn->GetActorLocation();
    const FVector TargetLocation = TargetActor->GetActorLocation();
    const FVector Direction = (TargetLocation - PawnLocation).GetSafeNormal2D();

    const FRotator TargetRot = Direction.Rotation();
    const FRotator NewRot = FMath::RInterpTo(Pawn->GetActorRotation(), TargetRot,
        Pawn->GetWorld()->GetDeltaSeconds(), InterpSpeed);

    Pawn->SetActorRotation(NewRot);
}

bool UMC_AINodeLibrary::HasLineOfSightToTarget(UObject* WorldContextObject, AAIController* Controller, AActor* TargetActor)
{
    if (!Controller || !TargetActor)
        return false;

    return Controller->LineOfSightTo(TargetActor);
}

void UMC_AINodeLibrary::DrawDebugLocation(UObject* WorldContextObject, const FVector& Location, float Radius, FColor Color, float Duration)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World)
        return;

    DrawDebugSphere(World, Location, Radius, 16, Color, false, Duration, 0, 1.5f);
}