// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "GengarAbilityLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

UAbilitySystemComponent* UGengarAbilityLibrary::ResolveAbilitySystemComponent(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return nullptr;
    }

    if (auto* Asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
    {
        return Asc;
    }

    if (const auto* Pawn = Cast<APawn>(Actor))
    {
        return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn->GetPlayerState());
    }

    if (const auto* Controller = Cast<AController>(Actor))
    {
        if (auto* Asc = ResolveAbilitySystemComponent(Controller->GetPawn()))
        {
            return Asc;
        }
        return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Controller->PlayerState);
    }

    return nullptr;
}

void UGengarAbilityLibrary::InitAbilityActorInfo(UAbilitySystemComponent* AbilitySystemComponent, AActor* Owner,
                                                 AActor* Avatar, APlayerController* PlayerController)
{
    AbilitySystemComponent->InitAbilityActorInfo(Owner, Avatar);

    if (PlayerController)
    {
        const auto OldPc = AbilitySystemComponent->AbilityActorInfo->PlayerController;
        if (OldPc.Get() != PlayerController)
        {
            AbilitySystemComponent->AbilityActorInfo->PlayerController = PlayerController;
            AbilitySystemComponent->OnPlayerControllerSet();
        }
    }
}

void UGengarAbilityLibrary::GetGameplayEffectContext(const FGameplayEffectContextHandle& ContextHandle,
                                                     FGengarGameplayEffectContext& Context,
                                                     bool& Valid)
{
    Valid = false;
    Context = {};
    if (ContextHandle.IsValid())
    {
        const auto* RawContext = ContextHandle.Get();
        if (RawContext->GetScriptStruct()->IsChildOf(FGengarGameplayEffectContext::StaticStruct()))
        {
            Context = *static_cast<const FGengarGameplayEffectContext*>(RawContext);
            Valid = true;
        }
    }
}

void UGengarAbilityLibrary::MakeEffectContextHandle(UAbilitySystemComponent* AbilitySystemComponent,
                                                    const FInstancedStruct& Payload,
                                                    FGameplayEffectContextHandle& Handle)
{
    if (!IsValid(AbilitySystemComponent))
    {
        Handle = {};
        return;
    }
    Handle = AbilitySystemComponent->MakeEffectContext();
    SetEffectContextPayload(Handle, Payload);
}

FInstancedStruct UGengarAbilityLibrary::TryGetEffectContextPayload(const FGameplayEffectContextHandle& Handle)
{
    bool bValid = false;
    FGengarGameplayEffectContext Context;
    GetGameplayEffectContext(Handle, Context, bValid);

    if (bValid)
    {
        return Context.Payload;
    }

    return {};
}

void UGengarAbilityLibrary::GetAbilityContextPayload(const UGameplayAbility* Ability,
                                                     FInstancedStruct& Payload)
{
    const auto EventDataProperty = UGameplayAbility::StaticClass()->FindPropertyByName("CurrentEventData");
    FGameplayEventData CurrentEventData;
    EventDataProperty->GetValue_InContainer(Ability, &CurrentEventData);

    FGengarGameplayEffectContext Context;
    bool bValid;
    GetGameplayEffectContext(CurrentEventData.ContextHandle, Context, bValid);
    if (bValid)
    {
        Payload = Context.Payload;
    }
}

void UGengarAbilityLibrary::SetEffectContextPayload(FGameplayEffectContextHandle& Handle,
                                                    const FInstancedStruct& Payload)
{
    auto* Context = Handle.Get();
    if (Context && Context->GetScriptStruct()->IsChildOf(FGengarGameplayEffectContext::StaticStruct()))
    {
        static_cast<FGengarGameplayEffectContext*>(Context)->Payload = Payload;
    }
}

FGameplayAbilitySpecHandle UGengarAbilityLibrary::FindAbilitySpecHandleByClass(
    const UAbilitySystemComponent* AbilitySystemComponent,
    const TSubclassOf<UGameplayAbility>& AbilityClass)
{
    if (!IsValid(AbilitySystemComponent) || !AbilityClass)
    {
        return {};
    }
    for (auto&& Spec : AbilitySystemComponent->GetActivatableAbilities())
    {
        if (Spec.Ability == AbilityClass.GetDefaultObject())
        {
            return Spec.Handle;
        }
    }

    return FGameplayAbilitySpecHandle();
}

bool UGengarAbilityLibrary::ActivateAbilityWithEventData(UAbilitySystemComponent* AbilitySystemComponent,
                                                         const FGameplayAbilitySpecHandle& Handle,
                                                         const FGameplayTag Tag,
                                                         const FGameplayEventData& EventData,
                                                         UGameplayAbility*& AbilityInstance,
                                                         FGengarAbilityEndedCallback OnEnded)
{
    AbilityInstance = nullptr;
    if (!IsValid(AbilitySystemComponent) || !AbilitySystemComponent->AbilityActorInfo.IsValid()
        || !Handle.IsValid() || !AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
    {
        return false;
    }

    const auto* ExistingSpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
    if (ExistingSpec->IsActive())
    {
        return false;
    }

    // Subscribe before activation: an instant ability may finish inside the activation call.
    auto EndedHandle = MakeShared<FDelegateHandle>();
    const auto WeakAsc = TWeakObjectPtr<UAbilitySystemComponent>(AbilitySystemComponent);
    if (OnEnded.IsBound())
    {
        *EndedHandle = AbilitySystemComponent->OnAbilityEnded.AddLambda(
            [WeakAsc, Handle, OnEnded, EndedHandle](const FAbilityEndedData& Data)
            {
                if (Data.AbilitySpecHandle != Handle)
                {
                    return;
                }
                if (auto* Asc = WeakAsc.Get())
                {
                    Asc->OnAbilityEnded.Remove(*EndedHandle);
                }
                OnEnded.ExecuteIfBound();
            });
    }

    const auto bActivated = AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
        Handle, AbilitySystemComponent->AbilityActorInfo.Get(), Tag, &EventData, *AbilitySystemComponent);
    if (!bActivated)
    {
        AbilitySystemComponent->OnAbilityEnded.Remove(*EndedHandle);
        return false;
    }

    if (const auto* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
    {
        const auto Instances = Spec->GetAbilityInstances();
        for (auto* Instance : Instances)
        {
            if (IsValid(Instance) && Instance->IsActive())
            {
                AbilityInstance = Instance;
            }
        }
    }
    return true;
}

bool UGengarAbilityLibrary::ActivateAbilityByClassWithEventData(UAbilitySystemComponent* AbilitySystemComponent,
                                                                const TSubclassOf<UGameplayAbility> AbilityClass,
                                                                const FGameplayTag Tag,
                                                                const FGameplayEventData& EventData,
                                                                UGameplayAbility*& AbilityInstance,
                                                                const FGengarAbilityEndedCallback OnEnded)
{
    return ActivateAbilityWithEventData(AbilitySystemComponent,
                                        FindAbilitySpecHandleByClass(AbilitySystemComponent, AbilityClass),
                                        Tag,
                                        EventData, AbilityInstance, OnEnded);
}
