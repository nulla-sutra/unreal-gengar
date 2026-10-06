// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GameplayAbilitySpecHandle.h"
#include "StructUtils/InstancedStruct.h"

#include "GengarStateTreeTask_TriggerAbility.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
struct FAbilityEndedData;

/**
 * 
 */
UCLASS(DisplayName = "Trigger Ability Once (Gengar)")
class GENGAR_API UGengarStateTreeTask_TriggerAbility : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:
    explicit UGengarStateTreeTask_TriggerAbility(const FObjectInitializer& ObjectInitializer):
        UStateTreeTaskBlueprintBase(
            ObjectInitializer)
    {
        bShouldCopyBoundPropertiesOnTick = false;
        bShouldCopyBoundPropertiesOnExitState = false;
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowAbstract = false))
    TSubclassOf<UGameplayAbility> Ability;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FInstancedStruct ContextPayload;

    // Existing combo abilities can outlive a state; interruption is an explicit choice.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCancelAbilityOnExit = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Output)
    bool bAbilityEnded = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Output)
    bool bWasCancelled = false;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private:
    void HandleAbilityEnded(const FAbilityEndedData& Data);
    void UnbindAbilityEnded();

    TWeakObjectPtr<UAbilitySystemComponent> BoundAbilitySystemComponent;
    TWeakObjectPtr<UGameplayAbility> ActiveAbility;
    FGameplayAbilitySpecHandle ActiveHandle;
    FDelegateHandle AbilityEndedHandle;
    bool bEnteringState = false;
};
