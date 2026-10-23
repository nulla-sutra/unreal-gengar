// Copyright 2019-Present tarnishablec. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gengar/ActorInfo/GengarAbilityAvatarInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "GengarGameplayAbilityActorInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct GENGAR_API FGengarGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
    GENERATED_BODY()

    virtual void InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor,
                               UAbilitySystemComponent* InAbilitySystemComponent) override
    {
        FGameplayAbilityActorInfo::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

        if (AvatarActor.IsValid()
            && (AvatarActor->Implements<UGengarAbilityAvatarInterface>()
                || Cast<IGengarAbilityAvatarInterface>(AvatarActor.Get())
            )
        )
        {
            SkeletalMeshComponent = IGengarAbilityAvatarInterface::Execute_AvatarMeshComponent(AvatarActor.Get());
        }
    }

    virtual void SetAvatarActor(AActor* InAvatarActor) override
    {
        FGameplayAbilityActorInfo::SetAvatarActor(InAvatarActor);
    }
};
