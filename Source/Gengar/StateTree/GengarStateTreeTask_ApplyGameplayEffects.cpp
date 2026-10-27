// Copyright 2019-Present tarnishablec. All Rights Reserved.

#include "Gengar/StateTree/GengarStateTreeTask_ApplyGameplayEffects.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayCue_Types.h"
#include "GameplayEffect.h"
#include "UObject/Class.h"

UGengarStateTreeTask_ApplyGameplayEffects::UGengarStateTreeTask_ApplyGameplayEffects(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bShouldCallTick = false;
    bShouldCopyBoundPropertiesOnTick = false;
    bShouldCopyBoundPropertiesOnExitState = false;
    bShouldStateChangeOnReselect = false;
#if WITH_EDITORONLY_DATA
    // Applying a side effect must neither finish the state nor block its action task's completion.
    bConsideredForCompletion = false;
    bCanEditConsideredForCompletion = false;
#endif
}

EStateTreeRunStatus UGengarStateTreeTask_ApplyGameplayEffects::EnterState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition)
{
    const auto EntrySerial = ++ActivationSerial;
    bActive = true;
    Super::EnterState(Context, Transition);
    if (!bActive || ActivationSerial != EntrySerial)
    {
        return EStateTreeRunStatus::Running;
    }

    // GE callbacks can stop or reenter the tree and replace this task's bound inputs.
    const auto EffectClasses = Effects;
    const auto EffectLevel = Level;
    const TWeakObjectPtr<UGameplayAbility> RunningAbility(Ability.Get());
    auto* OwnerAbility = RunningAbility.Get();
    if (!IsValid(OwnerAbility) || !OwnerAbility->IsInstantiated() || !OwnerAbility->IsActive()
        || !FMath::IsFinite(EffectLevel) || EffectLevel < 0.f)
    {
        return EStateTreeRunStatus::Failed;
    }
    const auto* ActorInfo = OwnerAbility->GetCurrentActorInfo();
    if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid() || !ActorInfo->OwnerActor.IsValid())
    {
        return EStateTreeRunStatus::Failed;
    }

    const auto ExpectedAsc = ActorInfo->AbilitySystemComponent;
    const auto ExpectedOwner = ActorInfo->OwnerActor;
    const auto ExpectedAvatar = ActorInfo->AvatarActor;
    const auto ExpectedHandle = OwnerAbility->GetCurrentAbilitySpecHandle();
    const auto ExpectedPredictionKey = OwnerAbility->GetCurrentActivationInfo().GetActivationPredictionKey();
    if (!ExpectedHandle.IsValid())
    {
        return EStateTreeRunStatus::Failed;
    }
    for (const auto& EffectClass : EffectClasses)
    {
        const auto* Class = EffectClass.Get();
        if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
        {
            return EStateTreeRunStatus::Failed;
        }
    }

    const auto bAuthority = ExpectedOwner->GetNetMode() != NM_Client && ExpectedOwner->HasAuthority();
    // Borrow the action's scoped key, never the activation key that may already have been acknowledged.
    const auto ActionPredictionKey = ExpectedAsc->GetPredictionKeyForNewAction();
    if (!bAuthority && (!ActorInfo->IsLocallyControlled() || !ActionPredictionKey.IsValidForMorePrediction()))
    {
        return EStateTreeRunStatus::Running;
    }

    const auto IsCurrentAbilityContext = [&]()
    {
        const auto* CurrentAbility = RunningAbility.Get();
        if (!IsValid(CurrentAbility) || !CurrentAbility->IsActive()
            || !ExpectedAsc.IsValid() || !ExpectedOwner.IsValid())
        {
            return false;
        }
        const auto* CurrentInfo = CurrentAbility->GetCurrentActorInfo();
        return CurrentInfo && CurrentInfo->AbilitySystemComponent == ExpectedAsc
            && CurrentInfo->OwnerActor == ExpectedOwner && CurrentInfo->AvatarActor == ExpectedAvatar
            && (bAuthority ? ExpectedOwner->HasAuthority() : CurrentInfo->IsLocallyControlled())
            && CurrentAbility->GetCurrentAbilitySpecHandle() == ExpectedHandle
            && CurrentAbility->GetCurrentActivationInfo().GetActivationPredictionKey() == ExpectedPredictionKey;
    };

    for (const auto& EffectClass : EffectClasses)
    {
        if (!IsCurrentAbilityContext())
        {
            return EStateTreeRunStatus::Failed;
        }

        // Use the ability's native path to preserve its context, source tags, SetByCaller values and spec hooks.
        const auto Spec = RunningAbility->MakeOutgoingGameplayEffectSpec(EffectClass, EffectLevel);
        if (!bActive || ActivationSerial != EntrySerial)
        {
            return EStateTreeRunStatus::Running;
        }
        if (!Spec.IsValid() || !IsCurrentAbilityContext())
        {
            return EStateTreeRunStatus::Failed;
        }
        auto* Asc = ExpectedAsc.Get();
        const auto ActivationInfo = RunningAbility->GetCurrentActivationInfo();
        if (!Asc->HasAuthorityOrPredictionKey(&ActivationInfo)
            || (!bAuthority && (!Asc->GetPredictionKeyForNewAction().IsValidForMorePrediction()
                || Asc->GetPredictionKeyForNewAction() != ActionPredictionKey)))
        {
            return EStateTreeRunStatus::Failed;
        }
        if (!bAuthority && Spec.Data->GetPeriod() > 0.f)
        {
            // GAS does not predict periodic effects; their authority application must not fail the local state.
            continue;
        }

        // The ability's apply wrapper is protected; preserve its cue batching and prediction key via public ASC APIs.
        FActiveGameplayEffectHandle Handle;
        {
            FScopedGameplayCueSendContext GameplayCueSendContext;
            Handle = Asc->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get(), ActionPredictionKey);
            if (Handle.WasSuccessfullyApplied() && ExpectedAsc.IsValid() && ExpectedOwner.IsValid() && bAuthority)
            {
                if (auto* ReplicationOwner = ExpectedAsc->GetOwner(); IsValid(ReplicationOwner))
                {
                    ReplicationOwner->ForceNetUpdate();
                }
            }
        }
        if (!bActive || ActivationSerial != EntrySerial)
        {
            return EStateTreeRunStatus::Running;
        }
        // Instant effects succeed without retaining a valid active-effect handle.
        if (!Handle.WasSuccessfullyApplied())
        {
            return EStateTreeRunStatus::Failed;
        }
    }

    return EStateTreeRunStatus::Running;
}

void UGengarStateTreeTask_ApplyGameplayEffects::ExitState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition)
{
    bActive = false;
    ++ActivationSerial;
    Super::ExitState(Context, Transition);
}
