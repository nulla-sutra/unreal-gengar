// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "GengarStateTreeTask_TriggerAbilityEachTick.h"

#include "Gengar/Helpers/GengarAbilityLibrary.h"

EStateTreeRunStatus UGengarStateTreeTask_TriggerAbilityEachTick::EnterState(FStateTreeExecutionContext& Context,
                                                                            const FStateTreeTransitionResult&
                                                                            Transition)
{
    return Super::EnterState(Context, Transition);
}

void UGengarStateTreeTask_TriggerAbilityEachTick::ExitState(FStateTreeExecutionContext& Context,
                                                            const FStateTreeTransitionResult& Transition)
{
    Super::ExitState(Context, Transition);
}

void UGengarStateTreeTask_TriggerAbilityEachTick::StateCompleted(FStateTreeExecutionContext& Context,
                                                                 const EStateTreeRunStatus CompletionStatus,
                                                                 const FStateTreeActiveStates& CompletedActiveStates)
{
    Super::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}

EStateTreeRunStatus UGengarStateTreeTask_TriggerAbilityEachTick::Tick(FStateTreeExecutionContext& Context,
                                                                      const float DeltaTime)
{
    if (!IsValid(AbilitySystemComponent) || !Ability || !AbilitySystemComponent->GetAvatarActor())
    {
        return EStateTreeRunStatus::Failed;
    }

    const auto Handle = UGengarAbilityLibrary::FindAbilitySpecHandleByClass(AbilitySystemComponent, Ability);
    const auto* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
    if (!Spec)
    {
        return EStateTreeRunStatus::Failed;
    }
    if (Spec->IsActive())
    {
        return EStateTreeRunStatus::Running;
    }

    FGameplayEventData EventData;
    UGameplayAbility* OutAbilityInstance = nullptr;
    UGengarAbilityLibrary::MakeEffectContextHandle(AbilitySystemComponent, ContextPayload, EventData.ContextHandle);
    UGengarAbilityLibrary::ActivateAbilityByClassWithEventData(
        AbilitySystemComponent, Ability,
        FGameplayTag::EmptyTag, EventData,
        OutAbilityInstance);

    return EStateTreeRunStatus::Running;
}
