// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Templates/SubclassOf.h"
#include "GengarStateTreeTask_ApplyGameplayEffects.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * Applies effects through the running ability once per state entry, on authority or in an existing owner prediction window.
 * No prediction window means clients skip the batch; periodic effects are skipped only on clients.
 * GAS owns stacking, duration, prediction reconciliation and replication; state exit does not remove effects.
 * Application failures stop the batch without undoing effects already applied.
 */
UCLASS(meta=(DisplayName="Apply Gameplay Effects (Gengar)", Category="Gengar"))
class GENGAR_API UGengarStateTreeTask_ApplyGameplayEffects : public UStateTreeTaskBlueprintBase
{
    GENERATED_BODY()

public:
    explicit UGengarStateTreeTask_ApplyGameplayEffects(const FObjectInitializer& ObjectInitializer);

    /** Bind the running Ability context; its ASC supplies the target and native effect replication. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Context)
    TObjectPtr<UGameplayAbility> Ability;

    /** Applied in order, including repeated classes; native GE stacking and refresh rules still apply. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Parameter, meta=(AllowAbstract="false"))
    TArray<TSubclassOf<UGameplayEffect>> Effects;

    /** Gameplay Effect level, independent of the ability level recorded in its effect context. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Parameter, meta=(ClampMin="0.0"))
    float Level = 1.f;

protected:
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
                                           const FStateTreeTransitionResult& Transition) override;
    virtual void ExitState(FStateTreeExecutionContext& Context,
                           const FStateTreeTransitionResult& Transition) override;

private:
    uint32 ActivationSerial = 0;
    bool bActive = false;
};
