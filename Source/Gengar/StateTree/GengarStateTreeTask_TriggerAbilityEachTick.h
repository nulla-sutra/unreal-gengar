// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "GengarStateTreeTask_TriggerAbilityEachTick.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "Trigger Ability Each Tick (Gengar)")
class GENGAR_API UGengarStateTreeTask_TriggerAbilityEachTick : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
    virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
                                const FStateTreeActiveStates& CompletedActiveStates) override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

public:
    explicit UGengarStateTreeTask_TriggerAbilityEachTick(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        bShouldCallTick = true;
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowAbstract = false))
    TSubclassOf<UGameplayAbility> Ability;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FInstancedStruct ContextPayload;
};
