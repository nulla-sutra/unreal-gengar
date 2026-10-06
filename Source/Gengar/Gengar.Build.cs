// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gengar : ModuleRules
{
    public Gengar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "AnimGraphRuntime",
                "GameplayAbilities",
                "GameplayTags",
                "StateTreeModule"
            }
        );
    }
}
