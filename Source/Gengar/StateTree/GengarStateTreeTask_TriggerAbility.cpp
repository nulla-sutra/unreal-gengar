// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "GengarStateTreeTask_TriggerAbility.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Gengar/Helpers/GengarAbilityLibrary.h"

EStateTreeRunStatus UGengarStateTreeTask_TriggerAbility::EnterState(FStateTreeExecutionContext& Context,
                                                                    const FStateTreeTransitionResult& Transition)
{
    Super::EnterState(Context, Transition);
    UnbindAbilityEnded();
    ActiveAbility.Reset();
    ActiveHandle = {};
    bAbilityEnded = false;
    bWasCancelled = false;

    if (!IsValid(AbilitySystemComponent) || !Ability
        || !AbilitySystemComponent->AbilityActorInfo.IsValid()
        || !AbilitySystemComponent->GetAvatarActor())
    {
        return EStateTreeRunStatus::Failed;
    }

    ActiveHandle = UGengarAbilityLibrary::FindAbilitySpecHandleByClass(AbilitySystemComponent, Ability);
    const auto* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(ActiveHandle);
    if (!Spec || Spec->IsActive())
    {
        // Never adopt or cancel an activation that another system already owns.
        ActiveHandle = {};
        return EStateTreeRunStatus::Failed;
    }

    BoundAbilitySystemComponent = AbilitySystemComponent;
    AbilityEndedHandle = AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &ThisClass::HandleAbilityEnded);
    TGuardValue EnteringGuard(bEnteringState, true);

    FGameplayEventData EventData;
    UGengarAbilityLibrary::MakeEffectContextHandle(AbilitySystemComponent, ContextPayload, EventData.ContextHandle);

    UGameplayAbility* OutAbilityInstance = nullptr;

    const auto bSuccess = UGengarAbilityLibrary::ActivateAbilityByClassWithEventData(
        AbilitySystemComponent, Ability,
        FGameplayTag::EmptyTag, EventData,
        OutAbilityInstance);

    if (!bSuccess)
    {
        UnbindAbilityEnded();
        ActiveHandle = {};
        return EStateTreeRunStatus::Failed;
    }

    ActiveAbility = OutAbilityInstance;
    if (bAbilityEnded)
    {
        return bWasCancelled ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Succeeded;
    }
    return EStateTreeRunStatus::Running;
}

void UGengarStateTreeTask_TriggerAbility::ExitState(FStateTreeExecutionContext& Context,
                                                    const FStateTreeTransitionResult& Transition)
{
    auto* Asc = BoundAbilitySystemComponent.Get();
    auto* Instance = ActiveAbility.Get();
    const auto Handle = ActiveHandle;
    UnbindAbilityEnded();
    ActiveHandle = {};
    ActiveAbility.Reset();
    if (bCancelAbilityOnExit && IsValid(Asc) && Asc->AbilityActorInfo.IsValid()
        && IsValid(Instance) && Instance->IsActive()
        && Instance->GetCurrentAbilitySpecHandle() == Handle)
    {
        Instance->CancelAbility(Handle, Asc->AbilityActorInfo.Get(), Instance->GetCurrentActivationInfo(), true);
    }
    Super::ExitState(Context, Transition);
}

void UGengarStateTreeTask_TriggerAbility::HandleAbilityEnded(const FAbilityEndedData& Data)
{
    if (Data.AbilitySpecHandle != ActiveHandle
        || (ActiveAbility.IsValid() && Data.AbilityThatEnded != ActiveAbility.Get()))
    {
        return;
    }
    bAbilityEnded = true;
    bWasCancelled = Data.bWasCancelled;
    UnbindAbilityEnded();
    if (!bEnteringState)
    {
        FinishTask(!bWasCancelled);
    }
}

void UGengarStateTreeTask_TriggerAbility::UnbindAbilityEnded()
{
    if (auto* Asc = BoundAbilitySystemComponent.Get())
    {
        Asc->OnAbilityEnded.Remove(AbilityEndedHandle);
    }
    AbilityEndedHandle.Reset();
    BoundAbilitySystemComponent.Reset();
}
