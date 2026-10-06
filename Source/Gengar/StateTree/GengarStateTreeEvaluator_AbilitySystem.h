// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "GengarStateTreeEvaluator_AbilitySystem.generated.h"

class UAbilitySystemComponent;
class USkeletalMeshComponent;
class AActor;
class AController;
class APawn;
class APlayerController;

USTRUCT()
struct GENGAR_API FGengarStateTreeEvaluator_AbilitySystem_InstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Input)
    TObjectPtr<AActor> Actor;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<AActor> AbilityOwner;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<AActor> Avatar;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<APawn> Pawn;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<AController> Controller;

    UPROPERTY(VisibleAnywhere, Category = Output)
    TObjectPtr<APlayerController> PlayerController;

    UPROPERTY(VisibleAnywhere, Category = Output)
    bool bReady = false;
};

/** Bind Actor to the stock component schema's Actor context. No GAS schema is required. */
USTRUCT(DisplayName = "Ability System (Gengar)")
struct GENGAR_API FGengarStateTreeEvaluator_AbilitySystem : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FGengarStateTreeEvaluator_AbilitySystem_InstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
    virtual void Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
    virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

private:
    static void Refresh(FInstanceDataType& Data);
};
