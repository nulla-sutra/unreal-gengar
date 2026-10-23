// Copyright 2019-Present tarnishablec. All Rights Reserved.


#include "Gengar/Globals/GengarAbilitySystemGlobals.h"
#include "Gengar/ActorInfo/GengarGameplayAbilityActorInfo.h"
#include "Gengar/Effect/GengarGameplayEffectContext.h"

FGameplayAbilityActorInfo* UGengarAbilitySystemGlobals::AllocAbilityActorInfo() const
{
    return new FGengarGameplayAbilityActorInfo();
}

FGameplayEffectContext* UGengarAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGengarGameplayEffectContext();
}

void UGengarAbilitySystemGlobals::InitGlobalData()
{
    Super::InitGlobalData();
}
