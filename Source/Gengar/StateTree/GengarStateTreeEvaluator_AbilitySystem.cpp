// Copyright 2019-Present tarnishablec. All Rights Reserved.

#include "Gengar/StateTree/GengarStateTreeEvaluator_AbilitySystem.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Gengar/Helpers/GengarAbilityLibrary.h"
#include "StateTreeExecutionContext.h"

void FGengarStateTreeEvaluator_AbilitySystem::Refresh(FInstanceDataType& Data)
{
    Data.AbilitySystemComponent = UGengarAbilityLibrary::ResolveAbilitySystemComponent(Data.Actor);
    Data.AbilityOwner = nullptr;
    Data.Avatar = nullptr;
    Data.Pawn = nullptr;
    Data.MeshComponent = nullptr;
    Data.Controller = nullptr;
    Data.PlayerController = nullptr;
    Data.bReady = false;

    const auto* Asc = Data.AbilitySystemComponent.Get();
    if (!IsValid(Asc) || !Asc->AbilityActorInfo.IsValid())
    {
        return;
    }

    const auto& Info = *Asc->AbilityActorInfo;
    Data.AbilityOwner = Info.OwnerActor.Get();
    Data.Avatar = Info.AvatarActor.Get();
    Data.Pawn = Cast<APawn>(Data.Avatar);
    Data.MeshComponent = Info.SkeletalMeshComponent.Get();
    Data.Controller = Info.PlayerController.Get();
    Data.PlayerController = Info.PlayerController.Get();
    if (!Data.Controller)
    {
        if (const auto* Pawn = Cast<APawn>(Data.Avatar))
        {
            Data.Controller = Pawn->GetController();
            Data.PlayerController = Cast<APlayerController>(Data.Controller);
        }
    }

    // Mesh and Controller are optional: GAS also runs on non-character actors.
    Data.bReady = IsValid(Data.AbilityOwner) && IsValid(Data.Avatar);
}

void FGengarStateTreeEvaluator_AbilitySystem::TreeStart(FStateTreeExecutionContext& Context) const
{
    Refresh(Context.GetInstanceData(*this));
}

void FGengarStateTreeEvaluator_AbilitySystem::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // Re-resolve on tree updates to avoid retaining a previous possession's ASC or Avatar.
    Refresh(Context.GetInstanceData(*this));
}

void FGengarStateTreeEvaluator_AbilitySystem::TreeStop(FStateTreeExecutionContext& Context) const
{
    auto& Data = Context.GetInstanceData(*this);
    Data.AbilitySystemComponent = nullptr;
    Data.AbilityOwner = nullptr;
    Data.Avatar = nullptr;
    Data.Pawn = nullptr;
    Data.MeshComponent = nullptr;
    Data.Controller = nullptr;
    Data.PlayerController = nullptr;
    Data.bReady = false;
}
