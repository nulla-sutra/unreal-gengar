// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gengar : ModuleRules
{
    public Gengar(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.Default;

        // Resolve module-qualified headers through Unreal's parent include paths.
        bLegacyParentIncludePaths = true;
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
